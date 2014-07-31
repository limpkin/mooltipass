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
#include <util/delay.h>
#include <stdlib.h>
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "eeprom_addresses.h"
#include "usb_cmd_parser.h"
//#include "had_mooltipass.h"
#include "userhandling.h"
#include "mooltipass.h"
#include "interrupts.h"
#include "smartcard.h"
#include "flash_mem.h"
#include "node_mgmt.h"
#include "node_test.h"
#include "defines.h"
#include "entropy.h"
#include "oledmp.h"
#include "utils.h"
#include "tests.h"
#include "touch.h"
#include "spi.h"
#include "pwm.h"
#include "usb.h"
#include "gui.h"

// Define the bootloader function
#ifdef AVR_BOOTLOADER_PROGRAMMING
    bootloader_f_ptr_type start_bootloader = (bootloader_f_ptr_type)0x3800; 
#endif
// Flag to inform if the caps lock timer is armed
volatile uint8_t wasCapsLockTimerArmed = FALSE;
// Caps lock timer
volatile uint16_t capsLockTimer = 0;


/*! \fn     disable_jtag(void)
*   \brief  Disable the JTAG module
*/
void disable_jtag(void)
{
    unsigned char temp;

    temp = MCUCR;
    temp |= (1<<JTD);
    MCUCR = temp;
    MCUCR = temp;
}

/*!	\fn		capsLockTick(void)
*	\brief	Function called every ms by interrupt
*/
void capsLockTick(void)
{
    if (capsLockTimer != 0)
    {
        capsLockTimer--;
    }
}

/*! \fn     main(void)
*   \brief  Main function
*/
int main(void)
{
    uint8_t temp_ctr_val[AES256_CTR_LENGTH];
    uint8_t usb_buffer[RAWHID_TX_SIZE];
    uint8_t* temp_buffer = usb_buffer;
    RET_TYPE flash_init_result;
    RET_TYPE touch_init_result;
    RET_TYPE card_detect_ret;
    RET_TYPE temp_rettype;
    uint8_t temp_user_id;

    /* Check if a card is inserted in the Mooltipass to go to the bootloader */
    #ifdef AVR_BOOTLOADER_PROGRAMMING
        disable_jtag();                 // Disable JTAG to gain access to pins
        DDR_SC_DET &= ~(1 << PORTID_SC_DET);
        PORT_SC_DET |= (1 << PORTID_SC_DET);
        _delay_ms(100);    
        #if defined(HARDWARE_V1)
        if (PIN_SC_DET & (1 << PORTID_SC_DET))
        #elif defined(HARDWARE_OLIVIER_V1)
        if (!(PIN_SC_DET & (1 << PORTID_SC_DET)))
        #endif
        {
            start_bootloader();
        }
    #endif

    CPU_PRESCALE(0);                    // Set for 16MHz clock
    _delay_ms(500);                     // Let the power settle
    disable_jtag();                     // Disable JTAG to gain access to pins
    initPortSMC();                      // Initialize smart card port
    initPwm();                          // Initialize PWM controller
    initIRQ();                          // Initialize interrupts    
    initUsb();                          // Initialize USB controller
    initI2cPort();                      // Initialize I2C interface
    entropyInit();                      // Initialize avrentropy library
    while(!isUsbConfigured());          // Wait for host to set configuration
    spiUsartBegin(SPI_RATE_8_MHZ);      // Start USART SPI at 8MHz

    // Launch the before flash initialization tests
    beforeFlashInitTests();
    
    // Check if we can initialize the Flash memory
    flash_init_result = initFlash();
    
    // Launch the after flash initialization tests
    afterFlashInitTests();
    
    // Set up OLED now that USB is receiving full 500mA.
    oledBegin(FONT_DEFAULT);

    // OLED screen is reversed on Olivier's design
    #ifdef HARDWARE_OLIVIER_V1
        oledSetRemap(OLED_REMAP_NIBBLES|OLED_REMAP_COL_ADDR);
    #endif
    
    oledSetColour(15);
    oledSetBackground(0);
    oledSetContrast(0xFF);
    oledSetScrollSpeed(3);
    oledWriteActiveBuffer();
    
    // First time initializations
    if (eeprom_read_word((uint16_t*)EEP_BOOTKEY_ADDR) != 0xDEAD)
    {
        eraseFlashUsersContents();
        firstTimeUserHandlingInit();
        eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, 0xDEAD);
    }
    
    // Check if we can initialize the touch sensing element
    touch_init_result = initTouchSensing();
    
    // Launch the after touch initialization tests
    afterTouchInitTests();
    
    // Test procedure to check that all HW is working
    //#define HW_TEST_PROC
    #ifdef HW_TEST_PROC
        oledSetXY(0,0);        
        if (flash_init_result == RETURN_OK)
        {
            printf_P(PSTR("FLASH OK\r\n"));
        } 
        else
        {
            printf_P(PSTR("PB FLASH\r\n"));
        }
        if (touch_init_result == RETURN_OK)
        {
            printf_P(PSTR("TOUCH OK\r\n"));            
        } 
        else
        {
            printf_P(PSTR("PB TOUCH\r\n"));
        }
        printf_P(PSTR("Bring hand close, touch left, wheel, right\r\n"));
        while(!(touchDetectionRoutine() & RETURN_PROX_DETECTION));
        printf_P(PSTR("Det, "));
        activateGuardKey();
        while(!(touchDetectionRoutine() & RETURN_LEFT_PRESSED));
        printf_P(PSTR("left, "));
        while(!(touchDetectionRoutine() & RETURN_WHEEL_PRESSED));
        printf_P(PSTR("wheel, "));
        while(!(touchDetectionRoutine() & RETURN_RIGHT_PRESSED));
        printf_P(PSTR("right!\r\n"));
        printf_P(PSTR("Insert card\r\n"));
        while(isCardPlugged() != RETURN_JDETECT);
        temp_rettype = cardDetectedRoutine();
        if ((temp_rettype == RETURN_MOOLTIPASS_BLANK) || (temp_rettype == RETURN_MOOLTIPASS_USER))
        {
            printf_P(PSTR("CARD OK\r\n"));
        } 
        else
        {
            printf_P(PSTR("PB CARD\r\n"));
        }
        printf_P(PSTR("Check LEDs!\r\n"));
        switchOnButtonWheelLeds();
        setPwmDc(MAX_PWM_VAL);
        if ((flash_init_result == RETURN_OK) && (touch_init_result == RETURN_OK) && ((temp_rettype == RETURN_MOOLTIPASS_BLANK) || (temp_rettype == RETURN_MOOLTIPASS_USER)))
        {
            printf_P(PSTR("---- TEST OK !!! ----\r\n"));
        }
        else
        {
            printf_P(PSTR("---- TEST NOT OK ----\r\n"));
        }
        while(1);
    #endif
    
    // Stop the Mooltipass if we can't communicate with the Flash
    if (flash_init_result != RETURN_OK)
    {
        oledSetXY(2,0);
        printf_P(PSTR("Problem flash init"));
        while(1);
    }
    
    // Display error message if we can't communicate with the touch sensing
    if (touch_init_result != RETURN_OK)
    {
        oledSetXY(2,0);
        printf_P(PSTR("Problem touch init"));
        delay_ms(2000);
    }

    // Write inactive buffer & default bitmap
    oledWriteInactiveBuffer();
    oledBitmapDrawFlash(0, 0, 0, OLED_SCROLL_UP);
        
    // Launch the after HaD logo display tests
    afterHadLogoDisplayTests();  
    
    // Let's fade in the LEDs
    for (uint16_t i = 0; i < MAX_PWM_VAL; i++)
    {
        setPwmDc(i);
        _delay_us(888);
    }
    activityDetectedRoutine();
    launchCalibrationCycle();
    
    while (1)
    {
        card_detect_ret = isCardPlugged();
        
        // Call GUI routine
        guiMainLoop();
        
        // Process possible incoming data
        if(usbRawHidRecv(usb_buffer, USB_READ_TIMEOUT) == RETURN_COM_TRANSF_OK)
        {
            usbProcessIncoming(usb_buffer);
        }  
        
        // Two quick caps lock presses wake up the device
        if ((capsLockTimer == 0) && (getKeyboardLeds() & HID_CAPS_MASK) && (wasCapsLockTimerArmed == FALSE))
        {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                wasCapsLockTimerArmed = TRUE;
                capsLockTimer = CAPS_LOCK_DEL;
            }
        }
        else if ((capsLockTimer != 0) && !(getKeyboardLeds() & HID_CAPS_MASK))
        {
            activityDetectedRoutine();
        }
        else if ((capsLockTimer == 0) && !(getKeyboardLeds() & HID_CAPS_MASK))
        {
            wasCapsLockTimerArmed = FALSE;            
        }
        
        if (card_detect_ret == RETURN_JDETECT)                          // Card just detected
        {
            temp_rettype = cardDetectedRoutine();
            activityDetectedRoutine();

            // Problem with card
            if ((temp_rettype == RETURN_MOOLTIPASS_PB) || (temp_rettype == RETURN_MOOLTIPASS_INVALID))
            {
                oledClear();
                oledPutstrXY_P(0, 28, OLED_CENTRE, PSTR("PB with card"));
                oledFlipBuffers(OLED_SCROLL_UP, 5);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                    // Shut down card reader
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_BLOCKED)         // Card blocked
            {
                oledClear();
                oledPutstrXY_P(0, 28, OLED_CENTRE, PSTR("Card blocked"));
                oledFlipBuffers(OLED_SCROLL_UP, 5);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                    // Shut down card reader
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_BLANK)           // Blank Mooltipass card
            {
                // Ask the user to setup his mooltipass card
                if (guiAskForConfirmation(PSTR("Create new mooltipass user?")) == RETURN_OK)
                {
                    // Create a new user with his new smart card
                    if (addNewUserAndNewSmartCard(SMARTCARD_DEFAULT_PIN) == RETURN_OK)
                    {
                        #ifdef GENERAL_LOGIC_OUTPUT_USB
                            usbPutstr_P(PSTR("New user and new card added\n"));
                        #endif
                        
                        oledClear();
                        oledPutstrXY_P(0, 28, OLED_CENTRE, PSTR("User added"));
                        oledFlipBuffers(OLED_SCROLL_UP, 5);
                        setSmartCardInsertedUnlocked();
                    }
                    else
                    {
                        #ifdef GENERAL_LOGIC_OUTPUT_USB
                            usbPutstr_P(PSTR("Couldn't add new user"));
                        #endif
                        
                        oledClear();
                        oledPutstrXY_P(0, 28, OLED_CENTRE, PSTR("Couldn't add"));
                        oledFlipBuffers(OLED_SCROLL_UP, 5);
                    }
                    _delay_ms(2000);
                    oledBitmapDrawFlash(0, 0, 0, OLED_SCROLL_UP);
                }
                printSMCDebugInfoToScreen();                            // Print smartcard info
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_USER)            // Configured mooltipass card
            {
                // Here we should ask the user for his pin and call mooltipassDetectedRoutine
                readCodeProtectedZone(temp_buffer);
                #ifdef GENERAL_LOGIC_OUTPUT_USB
                    usbPrintf_P(PSTR("%d cards\r\n"), getNumberOfKnownCards());
                    usbPrintf_P(PSTR("%d users\r\n"), getNumberOfKnownUsers());
                #endif
                
                // See if we know the card and if so fetch the user id & CTR nonce
                if (getUserIdFromSmartCardCPZ(temp_buffer, temp_ctr_val, &temp_user_id) == RETURN_OK)
                {
                    #ifdef GENERAL_LOGIC_OUTPUT_USB
                        usbPrintf_P(PSTR("Card ID found with user %d\r\n"), temp_user_id);
                    #endif
                    
                    // Developer mode, enter default pin code
                    #ifdef NO_PIN_CODE_REQUIRED
                        mooltipassDetectedRoutine(SMARTCARD_DEFAULT_PIN);
                        readAES256BitsKey(temp_buffer);
                        initUserFlashContext(temp_user_id);
                        initEncryptionHandling(temp_buffer, temp_ctr_val);
                        setSmartCardInsertedUnlocked();
                        oledClear();
                        oledPutstrXY_P(0, 28, OLED_CENTRE, PSTR("Card unlocked"));
                        oledFlipBuffers(OLED_SCROLL_UP, 5);
                        _delay_ms(2000);
                        oledBitmapDrawFlash(0, 0, 0, OLED_SCROLL_UP);
                    #endif
                }
                else
                {
                    oledClear();
                    oledPutstrXY_P(0, 28, OLED_CENTRE, PSTR("Card ID not found"));
                    oledFlipBuffers(OLED_SCROLL_UP, 5);
                    _delay_ms(2000);
                    oledBitmapDrawFlash(0, 0, 0, OLED_SCROLL_UP);
                    
                    // Developer mode, enter default pin code
                    #ifdef NO_PIN_CODE_REQUIRED
                        mooltipassDetectedRoutine(SMARTCARD_DEFAULT_PIN);
                        setSmartCardInsertedUnlocked();
                    #else
                        removeFunctionSMC();                            // Shut down card reader
                    #endif
                    
                    #ifdef GENERAL_LOGIC_OUTPUT_USB
                        usbPutstr_P(PSTR("Card ID not found\r\n"));
                    #endif
                }
                printSMCDebugInfoToScreen();
            }
        }
        else if (card_detect_ret == RETURN_JRELEASED)                   // Card just released
        {
            oledBitmapDrawFlash(0, 0, 0, OLED_SCROLL_UP);
        }
    }
}
