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
#include "eeprom_addresses.h"
#include "watchdog_driver.h"
#include "logic_smartcard.h"
#include "usb_cmd_parser.h"
#include "timer_manager.h"
#include "logic_eeprom.h"
#include "mooltipass.h"
#include "interrupts.h"
#include "smartcard.h"
#include "flash_mem.h"
#include "defines.h"
#include "oledmp.h"
#include "delays.h"
#include "utils.h"
#include "tests.h"
#include "touch.h"
#include "anim.h"
#include "spi.h"
#include "pwm.h"
#include "usb.h"
#include "rng.h"

// Define the bootloader function
bootloader_f_ptr_type start_bootloader = (bootloader_f_ptr_type)0x3800;
// Flag to inform if the caps lock timer is armed
volatile uint8_t wasCapsLockTimerArmed = FALSE;
// Boolean to know if user timeout is enabled
uint8_t mp_timeout_enabled = FALSE;


/*! \fn     disableJTAG(void)
*   \brief  Disable the JTAG module
*/
static inline void disableJTAG(void)
{
    unsigned char temp;

    temp = MCUCR;
    temp |= (1<<JTD);
    MCUCR = temp;
    MCUCR = temp;
}

/*! \fn     smallForLoopBasedDelay(void)
*   \brief  Small delay used at the mooltipass start
*/
void smallForLoopBasedDelay(void)
{
    for (uint16_t i = 0; i < 2000; i++) asm volatile ("NOP");
}

/*! \fn     main(void)
*   \brief  Main function
*/
int main(void)
{
    uint16_t current_bootkey_val = eeprom_read_word((uint16_t*)EEP_BOOTKEY_ADDR);
    RET_TYPE flash_init_result;
    RET_TYPE touch_init_result;
    RET_TYPE card_detect_ret;
    uint8_t fuse_ok = TRUE;
    
    // Disable JTAG to gain access to pins, set prescaler to 1 (fuses not set)
    #ifndef PRODUCTION_KICKSTARTER_SETUP
        disableJTAG();
        CPU_PRESCALE(0);
    #endif
        
    // Check fuse settings: boot reset vector, 2k words, SPIEN, BOD 4.3V, programming & ver disabled >> http://www.engbedded.com/fusecalc/
    if ((boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS) != 0xFF) || (boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS) != 0xD8) || (boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS) != 0xF8) || (boot_lock_fuse_bits_get(GET_LOCK_BITS) != 0xFC))
    {
        fuse_ok = FALSE;
    }
    
    // Check if PB5 is low to start electrical test
    DDRB &= ~(1 << 5); PORTB |= (1 << 5);
    smallForLoopBasedDelay();
    if (!(PINB & (1 << 5)))
    {
        // Test result, true by default
        uint8_t test_result = TRUE;
        // Leave flash nS off
        DDR_FLASH_nS |= (1 << PORTID_FLASH_nS);
        PORT_FLASH_nS |= (1 << PORTID_FLASH_nS);
        // Set PORTD as output, leave PORTID_OLED_SS high
        DDRD |= 0xFF; PORTD |= 0xFF;
        // All other pins are input by default, run our test
        for (uint8_t i = 0; i < 4; i++)
        {
            PORTD |= 0xFF;
            smallForLoopBasedDelay();
            if (!(PINF & (0xC3)) || !(PINC & (1 << 6)) || !(PINE & (1 << 6)) || !(PINB & (1 << 4)))
            {
                test_result = FALSE;
            }
            PORTD &= (1 << PORTID_OLED_SS);
            smallForLoopBasedDelay();
            if ((PINF & (0xC3)) || (PINC & (1 << 6)) || (PINE & (1 << 6)) || (PINB & (1 << 4)))
            {
                test_result = FALSE;
            }
        }               
        // PB6 as test result output
        DDRB |= (1 << 6);
        // If test successful, light green LED
        if ((test_result == TRUE) && (fuse_ok == TRUE))
        {
            PORTB |= (1 << 6);
        } 
        else
        {
            PORTB &= ~(1 << 6);
        }
        while(1);
    }
    
    // This code will only be used for developers and beta testers
    #if !defined(PRODUCTION_SETUP) && !defined(PRODUCTION_KICKSTARTER_SETUP)
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
        // Check if there was a change in the mooltipass setting storage to reset the parameters to their correct values
        if (getMooltipassParameterInEeprom(USER_PARAM_INIT_KEY_PARAM) != USER_PARAM_CORRECT_INIT_KEY)
        {
            mooltipassParametersInit();
            setMooltipassParameterInEeprom(USER_PARAM_INIT_KEY_PARAM, USER_PARAM_CORRECT_INIT_KEY);
        }
    #endif

    // First time initializations for Eeprom (first boot at production or flash layout changes for beta testers)
    if (current_bootkey_val != CORRECT_BOOTKEY)
    {
        // Erase Mooltipass parameters
        mooltipassParametersInit();
        // Set bootloader password bool to FALSE
        eeprom_write_byte((uint8_t*)EEP_BOOT_PWD_SET, FALSE);
    }

    /* Check if a card is inserted in the Mooltipass to go to the bootloader */
    #ifdef AVR_BOOTLOADER_PROGRAMMING
        /* Disable JTAG to get access to the pins */
        disableJTAG();
        /* Init SMC port */
        initPortSMC();
        /* Delay for detection */
        smallForLoopBasedDelay();
        #if defined(HARDWARE_V1)
        if (PIN_SC_DET & (1 << PORTID_SC_DET))
        #elif defined(HARDWARE_OLIVIER_V1)
        if (!(PIN_SC_DET & (1 << PORTID_SC_DET)))
        #endif
        {
            uint16_t tempuint16;
            /* What follows is a copy from firstDetectFunctionSMC() */
            /* Enable power to the card */
            PORT_SC_POW &= ~(1 << PORTID_SC_POW);
            /* Default state: PGM to 0 and RST to 1 */
            PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
            DDR_SC_PGM |= (1 << PORTID_SC_PGM);
            PORT_SC_RST |= (1 << PORTID_SC_RST);
            DDR_SC_RST |= (1 << PORTID_SC_RST);
            /* Activate SPI port */
            PORT_SPI_NATIVE &= ~((1 << SCK_SPI_NATIVE) | (1 << MOSI_SPI_NATIVE));
            DDRB |= (1 << SCK_SPI_NATIVE) | (1 << MOSI_SPI_NATIVE);
            setSPIModeSMC();
            /* Let the card come online */
            smallForLoopBasedDelay();
            /* Check smart card FZ */
            readFabricationZone((uint8_t*)&tempuint16);
            if ((swap16(tempuint16)) != SMARTCARD_FABRICATION_ZONE)
            {
                removeFunctionSMC();
                start_bootloader();
            }
            else
            {
                removeFunctionSMC();
            }
        }
    #endif

    initPortSMC();                      // Initialize smart card port
    initPwm();                          // Initialize PWM controller
    initIRQ();                          // Initialize interrupts
    powerSettlingDelay();               // Let the power settle   
    initUsb();                          // Initialize USB controller
    powerSettlingDelay();               // Let the USB 3.3V LDO rise
    initI2cPort();                      // Initialize I2C interface
    rngInit();                          // Initialize avrentropy library
    oledInitIOs();                      // Initialize OLED input/outputs
    spiUsartBegin(SPI_RATE_8_MHZ);      // Start USART SPI at 8MHz

    // If offline mode isn't enabled, wait for device to be enumerated
    if (getMooltipassParameterInEeprom(OFFLINE_MODE_PARAM) == FALSE)
    {
        while(!isUsbConfigured());      // Wait for host to set configuration
    }    
    
    // Set correct timeout_enabled val
    mp_timeout_enabled = getMooltipassParameterInEeprom(LOCK_TIMEOUT_ENABLE_PARAM);

    // Launch the before flash initialization tests
    #ifdef TESTS_ENABLED
        beforeFlashInitTests();
    #endif
    
    // Check if we can initialize the Flash memory
    flash_init_result = initFlash();
    
    // Launch the after flash initialization tests
    #ifdef TESTS_ENABLED
        afterFlashInitTests();
    #endif
    
    // Set up OLED now that USB is receiving full 500mA.
    oledBegin(FONT_DEFAULT);
    
    // First time initializations for Flash (first time power up at production)
    if (current_bootkey_val != CORRECT_BOOTKEY)
    {
        // Erase everything non graphic in flash
        eraseFlashUsersContents();
        // Erase # of cards and # of users
        firstTimeUserHandlingInit();
    }
    
    // Check if we can initialize the touch sensing element
    touch_init_result = initTouchSensing();

    // Enable proximity detection
    #ifndef HARDWARE_V1
        activateProxDetection();
    #endif
    
    // Launch the after touch initialization tests
    #ifdef TESTS_ENABLED
        afterTouchInitTests();
    #endif
    
    // Test procedure to check that all HW is working
    #if defined(PRODUCTION_SETUP) || defined(PRODUCTION_KICKSTARTER_SETUP)
        if (current_bootkey_val != CORRECT_BOOTKEY)
        {
            RET_TYPE temp_rettype;        
            // Wait for USB host to upload bundle, which then sets USER_PARAM_INIT_KEY_PARAM
            //#ifdef PRODUCTION_KICKSTARTER_SETUP
            while(getMooltipassParameterInEeprom(USER_PARAM_INIT_KEY_PARAM) != 0xF1)
            {
                usbProcessIncoming(USB_CALLER_MAIN);
            }
            //#endif
            // Bundle uploaded, start the screen
            oledBegin(FONT_DEFAULT);
            oledWriteActiveBuffer();
            oledSetXY(0,0);
            // LEDs ON, to check
            setPwmDc(MAX_PWM_VAL);
            switchOnButtonWheelLeds();
            guiDisplayRawString(ID_STRING_TEST_LEDS_CH);
            // Check flash init
            if (flash_init_result != RETURN_OK)
            {
                 guiDisplayRawString(ID_STRING_TEST_FLASH_PB);
            }
            // Check touch init
            if (touch_init_result != RETURN_OK)
            {
                guiDisplayRawString(ID_STRING_TEST_TOUCH_PB);
            }
            // Touch instructions
            guiDisplayRawString(ID_STRING_TEST_INST_TCH);
            // Check prox
            while(!(touchDetectionRoutine(0) & RETURN_PROX_DETECTION));
            guiDisplayRawString(ID_STRING_TEST_DET);
            activateGuardKey();
            // Check left
            while(!(touchDetectionRoutine(0) & RETURN_LEFT_PRESSED));
            guiDisplayRawString(ID_STRING_TEST_LEFT);
            // Check wheel
            while(!(touchDetectionRoutine(0) & RETURN_WHEEL_PRESSED));
            guiDisplayRawString(ID_STRING_TEST_WHEEL);
            // Check right
            while(!(touchDetectionRoutine(0) & RETURN_RIGHT_PRESSED));
            guiDisplayRawString(ID_STRING_TEST_RIGHT);
            // Insert card
            guiDisplayRawString(ID_STRING_TEST_CARD_INS);
            while(isCardPlugged() != RETURN_JDETECT);
            temp_rettype = cardDetectedRoutine();
            // Check card
            if (!((temp_rettype == RETURN_MOOLTIPASS_BLANK) || (temp_rettype == RETURN_MOOLTIPASS_USER)))
            {
                guiDisplayRawString(ID_STRING_TEST_CARD_PB);
            }
            // Display result
            uint8_t script_return = RETURN_OK;
            if ((flash_init_result == RETURN_OK) && (touch_init_result == RETURN_OK) && ((temp_rettype == RETURN_MOOLTIPASS_BLANK) || (temp_rettype == RETURN_MOOLTIPASS_USER)))
            {
                // Inform script of success
                usbSendMessage(CMD_FUNCTIONAL_TEST_RES, 1, &script_return);
                // Wait for password to be set
                while(eeprom_read_byte((uint8_t*)EEP_BOOT_PWD_SET) != BOOTLOADER_PWDOK_KEY)
                {
                    usbProcessIncoming(USB_CALLER_MAIN);
                }
                // Display test result
                guiDisplayRawString(ID_STRING_TEST_OK);
                timerBasedDelayMs(3000);
            }
            else
            {
                // Set correct bool
                script_return = RETURN_NOK;
                // Display test result
                guiDisplayRawString(ID_STRING_TEST_NOK);
                // Inform script of failure
                usbSendMessage(CMD_FUNCTIONAL_TEST_RES, 1, &script_return);
                while(1)
                {
                    usbProcessIncoming(USB_CALLER_MAIN);
                }
            }
        }
    #endif
    
    // Stop the Mooltipass if we can't communicate with the flash or the touch interface
    #if defined(HARDWARE_OLIVIER_V1)
        #ifdef PRODUCTION_KICKSTARTER_SETUP
            while ((flash_init_result != RETURN_OK) || (touch_init_result != RETURN_OK) || (fuse_ok != TRUE));
        #else
            while ((flash_init_result != RETURN_OK) || (touch_init_result != RETURN_OK));
        #endif
    #endif
    
    // First time initializations done.... write correct value in eeprom
    if (current_bootkey_val != CORRECT_BOOTKEY)
    {
        // Store correct bootkey
        eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, CORRECT_BOOTKEY);
    }

    // Write inactive buffer & go to startup screen
    oledWriteInactiveBuffer();
    guiSetCurrentScreen(SCREEN_DEFAULT_NINSERTED);
    guiGetBackToCurrentScreen();
        
    // Launch the after HaD logo display tests
    #ifdef TESTS_ENABLED
        afterHadLogoDisplayTests();  
    #endif
    
    // Let's fade in the LEDs
    for (uint16_t i = 0; i < MAX_PWM_VAL; i++)
    {
        setPwmDc(i);
        timerBasedDelayMs(0);
    }
    activityDetectedRoutine();
    launchCalibrationCycle();
    touchClearCurrentDetections();
    
    // Inhibit touch inputs for the first 3 seconds
    activateTimer(TIMER_TOUCH_INHIBIT, 3000);
    while (1)
    {
        // Process possible incoming USB packets
        usbProcessIncoming(USB_CALLER_MAIN);
        
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
            guiDisplayInformationOnScreen(ID_STRING_CARD_REMOVED);
            guiSetCurrentScreen(SCREEN_DEFAULT_NINSERTED);
            userViewDelay();
            guiGetBackToCurrentScreen();
        }
        
        // Two quick caps lock presses wakes up the device        
        if ((hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_EXPIRED) && (getKeyboardLeds() & HID_CAPS_MASK) && (wasCapsLockTimerArmed == FALSE))
        {
            wasCapsLockTimerArmed = TRUE;
            activateTimer(TIMER_CAPS, CAPS_LOCK_DEL);
        }
        else if ((hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_RUNNING) && !(getKeyboardLeds() & HID_CAPS_MASK))
        {
            activityDetectedRoutine();
            if (isScreenSaverOn() == TRUE)
            {
                guiGetBackToCurrentScreen();
            }
        }
        else if ((hasTimerExpired(TIMER_CAPS, FALSE) == TIMER_EXPIRED) && !(getKeyboardLeds() & HID_CAPS_MASK))
        {
            wasCapsLockTimerArmed = FALSE;            
        }
        
        // If we have a timeout lock
        if ((mp_timeout_enabled == TRUE) && (hasTimerExpired(SLOW_TIMER_LOCKOUT, TRUE) == TIMER_EXPIRED))
        {
            guiSetCurrentScreen(SCREEN_DEFAULT_INSERTED_LCK);
            guiGetBackToCurrentScreen();
            handleSmartcardRemoved();
        }
    }
}
