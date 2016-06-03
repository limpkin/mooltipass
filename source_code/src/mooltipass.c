/* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*! \file   mooltipass.c
 *  \brief  main file
 *  Copyright [2014] [Mathieu Stephan]
 */
#include <util/atomic.h>
#include <avr/eeprom.h>
#include <avr/boot.h>
#include <stdlib.h>
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "gui_smartcard_functions.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "logic_aes_and_comms.h"
#include "functional_testing.h"
#include "eeprom_addresses.h"
#include "define_printouts.h"
#include "watchdog_driver.h"
#include "logic_smartcard.h"
#include "usb_cmd_parser.h"
#include "timer_manager.h"
#include "bitstreammini.h"
#include "oled_wrapper.h"
#include "logic_eeprom.h"
#include "mini_inputs.h"
#include "mooltipass.h"
#include "interrupts.h"
#include "smartcard.h"
#include "mini_leds.h"
#include "flash_mem.h"
#include "defines.h"
#include "delays.h"
#include "utils.h"
#include "tests.h"
#include "touch.h"
#include "anim.h"
#include "spi.h"
#include "pwm.h"
#include "usb.h"
#include "rng.h"

#if !defined(MINI_VERSION)
// Tutorial led masks and touch filtering
static const uint8_t tutorial_masks[] __attribute__((__progmem__)) =
{
    0,                              TOUCH_PRESS_MASK,       // Welcome screen
    LED_MASK_WHEEL,                 RETURN_RIGHT_PRESSED,   // Show you around...
    LED_MASK_WHEEL,                 RETURN_RIGHT_PRESSED,   // Display hints
    LED_MASK_LEFT|LED_MASK_RIGHT,   RETURN_WHEEL_PRESSED,   // Circular segments
    LED_MASK_LEFT|LED_MASK_RIGHT,   RETURN_WHEEL_PRESSED,   // Wheel interface
    0,                              TOUCH_PRESS_MASK,       // That's all!
};
#endif
// Define the bootloader function
bootloader_f_ptr_type start_bootloader = (bootloader_f_ptr_type)0x3800;
// Flag to inform if the caps lock timer is armed
volatile uint8_t wasCapsLockTimerArmed = FALSE;
// Boolean to know if user timeout is enabled
uint8_t mp_timeout_enabled = FALSE;
// Flag set by anything to signal activity
uint8_t act_detected_flag = FALSE;


/*! \fn     main(void)
*   \brief  Main function
*/
int main(void)
{
    uint16_t current_bootkey_val = eeprom_read_word((uint16_t*)EEP_BOOTKEY_ADDR);   // Fetch boot key from EEPROM
    #if defined(HARDWARE_OLIVIER_V1)                                                // Only the Mooltipass standard version has a touch panel
        RET_TYPE touch_init_result;                                                 // Touch initialization result
    #endif                                                                          // ENDIF
    #if defined(MINI_VERSION)                                                       // Dedicated to mooltipass mini
        RET_TYPE mini_inputs_result;                                                // Mooltipass mini input init result
    #endif                                                                          // ENDIF
    RET_TYPE flash_init_result;                                                     // Flash initialization result
    RET_TYPE card_detect_ret;                                                       // Card detection result
    uint8_t fuse_ok = TRUE;                                                         // Fuse check result
    
    /********************************************************************/
    /**                     JTAG FUSE ACTIONS                          **/
    /*                                                                  */
    /* On units where the fuses aren't programmed, the JTAG is enabled  */
    /* by default, preventing the use of certain IOs. Moreover, the     */
    /* CKDIV8 is also set, which divides the clock by 8.                */
    /********************************************************************/
    #if defined(JTAG_FUSE_ENABLED)                                                  // For units whose fuses haven't been programmed
        disableJTAG();                                                              // Disable JTAG to gain access to pins        
        CPU_PRESCALE(0);                                                            // Set pre-scaler to 1 (fuses not set)
    #endif

    /********************************************************************/
    /**                 CHECK FOR BOOTLOADER BRICK                     **/
    /*                                                                  */
    /* On the Mooltipass mini, the bootloader may deliberately brick    */
    /* the device if a malicious attempt has been made.                 */
    /********************************************************************/
    #if defined(MINI_VERSION)
        if (current_bootkey_val == BRICKED_BOOTKEY)
        {
            while(1);
        }
    #endif

    /********************************************************************/
    /**                    FUSE VERIFICATIONS                          **/
    /*                                                                  */
    /* There's no point in letting the Mooltipass boot if the fuses     */
    /* aren't correctly set                                             */
    /********************************************************************/
    #if defined(MINI_CLICK_BETATESTERS_SETUP)
        // no fuse verification for the beta testers units
    #elif defined(PREPRODUCTION_KICKSTARTER_SETUP)
        // 2k words, SPIEN, BOD 4.3V, programming & ver disabled >> http://www.engbedded.com/fusecalc/
        if ((boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS) != 0xFF) || (boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS) != 0xD9) || (boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS) != 0xF8) || (boot_lock_fuse_bits_get(GET_LOCK_BITS) != 0xFC))
        {
            fuse_ok = FALSE;
        }
    #elif defined(PRODUCTION_TEST_SETUP)
        // 2k words, SPIEN, BOD 4.3V, no checks on programming fuses >> http://www.engbedded.com/fusecalc/
        if ((boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS) != 0xFF) || (boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS) != 0xD9) || (boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS) != 0xF8))
        {
            fuse_ok = FALSE;
        }
    #else
        // boot reset vector, 2k words, SPIEN, BOD 4.3V, programming & ver disabled >> http://www.engbedded.com/fusecalc/
        if ((boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS) != 0xFF) || (boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS) != 0xD8) || (boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS) != 0xF8) || (boot_lock_fuse_bits_get(GET_LOCK_BITS) != 0xFC))
        {
            fuse_ok = FALSE;
        }
    #endif
    
    /********************************************************************/
    /**                    ELECTRICAL TESTING                          **/
    /*                                                                  */
    /* The standard Mooltipass goes through electrical testing to make  */
    /* sure that all the MCU IOs are correctly soldered.                */
    /********************************************************************/
    #if defined(HARDWARE_OLIVIER_V1) && !defined(POST_KICKSTARTER_UPDATE_SETUP)
        mooltipassStandardElectricalTest(fuse_ok);
    #endif    
    
    /********************************************************************/
    /**               JUMPING TO BOOTLOADER AT BOOT                    **/
    /*                                                                  */
    /* In the non-production units that don't have the boot reset vector*/
    /* enabled, the testers may choose to start the bootloader by       */
    /* sending a USB command. Internally, we therefore reboot the MCU   */
    /* and then jump to the bootloader.                                 */
    /********************************************************************/
    #if !defined(PRODUCTION_SETUP) && !defined(PRODUCTION_KICKSTARTER_SETUP) && !defined(POST_KICKSTARTER_UPDATE_SETUP) && !defined(MINI_PREPRODUCTION_SETUP) && !defined(MINI_PREPRODUCTION_SETUP_ACC)
        // This code will only be used for developers and beta testers
        // Check if we were reset and want to go to the bootloader
        if (current_bootkey_val == BOOTLOADER_BOOTKEY)
        {
            // Disable WDT
            wdt_reset();
            wdt_clear_flag();
            wdt_change_enable();
            wdt_stop();
            // Store correct bootkey
            eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, CORRECT_BOOTKEY);
            // Jump to bootloader
            start_bootloader();
        }
    #endif    

    /********************************************************************/
    /**            EEPROM INITIALIZATIONS AT FIRST BOOT                **/
    /*                                                                  */
    /* During the first boot the Mooltipass settings stored in eeprom   */
    /* are set to unknown values. Here we set them to their defaults.   */
    /********************************************************************/
    if (current_bootkey_val != CORRECT_BOOTKEY)
    {
        // Erase Mooltipass parameters
        mooltipassParametersInit();
        // Set bootloader password bool to FALSE
        eeprom_write_byte((uint8_t*)EEP_BOOT_PWD_SET, FALSE);
    }
        
    /********************************************************************/
    /**            CHANGE IN MOOLTIPASS SETTINGS STORAGE               **/
    /*                                                                  */
    /* An easy but not often used way to reset the Mooltipass settings  */
    /********************************************************************/
    if (getMooltipassParameterInEeprom(USER_PARAM_INIT_KEY_PARAM) != USER_PARAM_CORRECT_INIT_KEY)
    {
        mooltipassParametersInit();
        setMooltipassParameterInEeprom(USER_PARAM_INIT_KEY_PARAM, USER_PARAM_CORRECT_INIT_KEY);
    }

   /********************************************************************/
   /**              JUMPING TO BOOTLOADER FOR TEST UNITS              **/
   /*                                                                  */
   /* On test units, a button pressed at boot starts the bootloader    */
   /********************************************************************/
    #ifdef AVR_BOOTLOADER_PROGRAMMING
        if(electricalJumpToBootloaderCondition() == TRUE)
        {
            start_bootloader();
        }
    #endif

    /** HARDWARE INITIALIZATION **/
    #if defined(HARDWARE_OLIVIER_V1) || defined(HARDWARE_MINI_CLICK_V2)
        initPwm();                              // Initialize PWM controller for MP standard & mini v2
    #endif                                      // ENDIF
    #if defined(HARDWARE_MINI_CLICK_V2)         // Only for the pre-production mini
        miniInitLeds();                         // Initialize the LEDs
    #endif                                      // ENDIF
    initPortSMC();                              // Initialize smart card port
    initIRQ();                                  // Initialize interrupts
    powerSettlingDelay();                       // Let the power settle before enabling USB controller
    initUsb();                                  // Initialize USB controller
    powerSettlingDelay();                       // Let the USB 3.3V LDO rise
    #if defined(HARDWARE_OLIVIER_V1)            // I2C is only used in the Mooltipass standard
        initI2cPort();                          // Initialize I2C interface
    #endif                                      // ENDIF
    rngInit();                                  // Initialize avrentropy library
    oledInitIOs();                              // Initialize OLED inputs/outputs
    initFlashIOs();                             // Initialize Flash inputs/outputs
    spiUsartBegin();                            // Start USART SPI at 8MHz (standard) or 4MHz (mini)
    #if defined(MINI_VERSION)                   // For the Mooltipass Mini inputs
        mini_inputs_result = initMiniInputs();  // Initialize Mini Inputs
    #endif                                      // ENDIF

    // If offline mode isn't enabled, wait for device to be enumerated
    if (getMooltipassParameterInEeprom(OFFLINE_MODE_PARAM) == FALSE)
    {
        while(!isUsbConfigured());              // Wait for host to set configuration
    }    
    
    // Set correct timeout_enabled val
    mp_timeout_enabled = getMooltipassParameterInEeprom(LOCK_TIMEOUT_ENABLE_PARAM);
    
    /** FLASH INITIALIZATION **/
    flash_init_result = checkFlashID();         // Check for flash presence
    
    /** OLED INITIALIZATION **/
    oledBegin(FONT_DEFAULT);                    // Only do it now as we're enumerated
    
    /** FIRST BOOT FLASH & EEPROM INITIALIZATIONS **/
    if (current_bootkey_val != CORRECT_BOOTKEY)
    {        
        chipErase();                            // Erase everything in flash        
        firstTimeUserHandlingInit();            // Erase # of cards and # of users
    }
    
    /** TOUCH PANEL INITIALIZATION **/
    #if defined(HARDWARE_OLIVIER_V1)
        touch_init_result = initTouchSensing();
        activateProxDetection();
    #endif
    
    /** FUNCTIONAL TESTING **/
    //#define FORCE_PROD_TEST
    #if defined(PRODUCTION_SETUP) || defined(PRODUCTION_KICKSTARTER_SETUP) || defined(FORCE_PROD_TEST)
        // Test procedure to check that all HW is working
        mooltipassStandardFunctionalTest(current_bootkey_val, flash_init_result, touch_init_result, fuse_ok);
    #endif
    #if defined(MINI_CLICK_BETATESTERS_SETUP) || defined(MINI_PREPRODUCTION_SETUP) ||  defined(MINI_PREPRODUCTION_SETUP_ACC)
        mooltipassMiniFunctionalTest(current_bootkey_val, flash_init_result, fuse_ok, mini_inputs_result);
    #endif
    
    /** BOOT STOP IF ERRORS **/
    #if defined(HARDWARE_OLIVIER_V1)
        #if defined(PRODUCTION_KICKSTARTER_SETUP) || defined(PREPRODUCTION_KICKSTARTER_SETUP) || defined(POST_KICKSTARTER_UPDATE_SETUP)
            while ((flash_init_result != RETURN_OK) || (touch_init_result != RETURN_OK) || (fuse_ok != TRUE));
        #else
            while ((flash_init_result != RETURN_OK) || (touch_init_result != RETURN_OK));
        #endif
    #elif defined(MINI_VERSION)
        #if defined(MINI_CLICK_BETATESTERS_SETUP)
            (void)fuse_ok;
            while ((flash_init_result != RETURN_OK) || (mini_inputs_result != RETURN_OK));
        #elif defined(MINI_PREPRODUCTION_SETUP)
            while ((flash_init_result != RETURN_OK) || (fuse_ok != TRUE) || (mini_inputs_result != RETURN_OK));
        #elif defined(MINI_PREPRODUCTION_SETUP_ACC)
            // TO REMOVE
            fuse_ok = TRUE;
            while ((flash_init_result != RETURN_OK) || (fuse_ok != TRUE) || (mini_inputs_result != RETURN_OK));
        #else
            #error "Platform unknown!"
        #endif
    #endif
    
    /** CORRECT BOOKEY WRITE **/
    if (current_bootkey_val != CORRECT_BOOTKEY)
    {
        eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, CORRECT_BOOTKEY);
    }
    
    // Write inactive buffer by default
    oledWriteInactiveBuffer();    
    
    // Display tutorial if needed
    if (getMooltipassParameterInEeprom(TUTORIAL_BOOL_PARAM) != FALSE)
    {
        #if defined(HARDWARE_OLIVIER_V1)
            uint8_t tut_led_mask, press_filter;
            activateGuardKey();
            activityDetectedRoutine();
            for (uint8_t i = 0; i < sizeof(tutorial_masks)/2; i++)
            {
                tut_led_mask = pgm_read_byte(&tutorial_masks[i*2]);
                press_filter = pgm_read_byte(&tutorial_masks[i*2+1]);
                oledBitmapDrawFlash(0, 0, i + BITMAP_TUTORIAL_1, OLED_SCROLL_UP);
                while(!(touchDetectionRoutine(tut_led_mask) & press_filter));
                touchInhibitUntilRelease();
            }
        #endif
        setMooltipassParameterInEeprom(TUTORIAL_BOOL_PARAM, FALSE);
    }

    // Go to startup screen
    guiSetCurrentScreen(SCREEN_DEFAULT_NINSERTED);
    guiGetBackToCurrentScreen();
    
    // LED fade-in for standard version & mini v2/3
    #if defined(HARDWARE_OLIVIER_V1)
        // Let's fade in the LEDs
        touchDetectionRoutine(0);
        for (uint16_t i = 0; i < MAX_PWM_VAL; i++)
        {
            setPwmDc(i);
            timerBasedDelayMs(0);
        }
        activityDetectedRoutine();
        launchCalibrationCycle();
        touchClearCurrentDetections();
    #elif defined(HARDWARE_MINI_CLICK_V2)
        miniLedsSetAnimation(ANIM_FADE_IN_FADE_OUT_1_TIME);
    #endif
    
    // Inhibit touch inputs for the first 2 seconds
    activateTimer(TIMER_TOUCH_INHIBIT, 2000);
    while (1)
    {
        // Process possible incoming USB packets
        usbProcessIncoming(USB_CALLER_MAIN);
        
        // Launch activity detected routine if flag is set
        if (act_detected_flag != FALSE)
        {
            if (isScreenSaverOn() == TRUE)
            {
                guiGetBackToCurrentScreen();
            }
            activityDetectedRoutine();
            act_detected_flag = FALSE;
        }
        
        // Call GUI routine once the touch input inhibit timer is finished
        if (hasTimerExpired(TIMER_TOUCH_INHIBIT, FALSE) == TIMER_EXPIRED)
        {
            guiMainLoop();
        }
        
        // If we are running the screen saver
        if (isScreenSaverOn() == TRUE)
        {
            animScreenSaver();
        }
        
        // If the USB bus is in suspend (computer went to sleep), lock device
        if ((hasTimerExpired(TIMER_USB_SUSPEND, TRUE) == TIMER_EXPIRED) && (getSmartCardInsertedUnlocked() == TRUE))
        {
            handleSmartcardRemoved();
            guiDisplayInformationOnScreenAndWait(ID_STRING_PC_SLEEP);
            guiSetCurrentScreen(SCREEN_DEFAULT_INSERTED_LCK);
            // If the screen saver is on, clear screen contents
            if(isScreenSaverOn() == TRUE)
            {
                #ifndef MINI_VERSION
                    oledClear();
                    oledDisplayOtherBuffer();
                    oledClear();
                #endif
            }
            else
            {
                guiGetBackToCurrentScreen();                
            }
        }
        
        // Check if a card just got inserted / removed
        card_detect_ret = isCardPlugged();
        
        // Do appropriate actions on smartcard insertion / removal
        if (card_detect_ret == RETURN_JDETECT)
        {
            // Light up the Mooltipass and call the dedicated function
            activityDetectedRoutine();
            handleSmartcardInserted();
        }
        else if (card_detect_ret == RETURN_JRELEASED)
        {
            // Light up the Mooltipass and call the dedicated function
            activityDetectedRoutine();
            handleSmartcardRemoved();
            
            // Set correct screen
            guiDisplayInformationOnScreenAndWait(ID_STRING_CARD_REMOVED);
            guiSetCurrentScreen(SCREEN_DEFAULT_NINSERTED);
            guiGetBackToCurrentScreen();
        }
        
        //#define TWO_CAPS_TRICK
        #ifdef TWO_CAPS_TRICK
        // Two quick caps lock presses wakes up the device        
        if ((hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_EXPIRED) && (getKeyboardLeds() & HID_CAPS_MASK) && (wasCapsLockTimerArmed == FALSE))
        {
            wasCapsLockTimerArmed = TRUE;
            activateTimer(TIMER_CAPS, CAPS_LOCK_DEL);
        }
        else if ((hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_RUNNING) && !(getKeyboardLeds() & HID_CAPS_MASK))
        {
            if (isScreenSaverOn() == TRUE)
            {
                guiGetBackToCurrentScreen();
            }
            activityDetectedRoutine();
        }
        else if ((hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_EXPIRED) && !(getKeyboardLeds() & HID_CAPS_MASK))
        {
            wasCapsLockTimerArmed = FALSE;            
        }
        #endif
        
        // If we have a timeout lock
        if ((mp_timeout_enabled == TRUE) && (hasTimerExpired(SLOW_TIMER_LOCKOUT, TRUE) == TIMER_EXPIRED))
        {
            guiSetCurrentScreen(SCREEN_DEFAULT_INSERTED_LCK);
            leaveMemoryManagementMode();
            guiGetBackToCurrentScreen();
            handleSmartcardRemoved();
        }
    }
}
