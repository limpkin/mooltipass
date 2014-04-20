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

#include <util/delay.h>
#include <stdlib.h>
#include <avr/io.h>
#include <stdio.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "had_mooltipass.h"
#include "usb_serial_hid.h"
#include "mooltipass.h"
#include "interrupts.h"
#include "smartcard.h"
#include "flash_mem.h"
#include "node_mgmt.h"
#include "defines.h"
#include "Entropy.h"
#include "oledmp.h"
#include "utils.h"
#include "tests.h"
#include "touch.h"
#include "spi.h"
#include "pwm.h"

#ifdef AVR_BOOTLOADER_PROGRAMMING
    bootloader_f_ptr_type start_bootloader = (bootloader_f_ptr_type)0x3800; 
#endif


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

/*! \fn     main(void)
*   \brief  Main function
*/
int main(void)
{
    RET_TYPE flash_init_result = RETURN_NOK;
    RET_TYPE touch_init_result = RETURN_NOK;
    RET_TYPE card_detect_ret;
    RET_TYPE temp_rettype;

    #ifdef AVR_BOOTLOADER_PROGRAMMING
        /* Check if a card is inserted in the Mooltipass to go to the bootloader */
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
    usb_init();                         // Initialize USB controller
    initI2cPort();                      // Initialize I2C interface
    entropyInit();                      // Initialize avrentropy library
    while(!usb_configured());           // Wait for host to set configuration
    spiUsartBegin(SPI_RATE_8_MHZ);      // Start USART SPI at 8MHz

    // Set up OLED now that USB is receiving full 500mA.
    oledBegin(FONT_DEFAULT);
    oledSetColour(15);
    oledSetBackground(0);
    oledSetContrast(0x8F);
    oledSetScrollSpeed(3);
    oledWriteActiveBuffer();
    
    // OLED screen is reversed on Olivier's design
    #ifdef HARDWARE_OLIVIER_V1
        oledSetRemap(OLED_REMAP_NIBBLES|OLED_REMAP_COL_ADDR);
    #endif
    
    beforeFlashInitTests();             // Launch the before flash init tests    
    flash_init_result = flashInit();    // Initialize flash memory
    afterFlashInitTests();              // Launch the after flash init tests

    // Stop the mooltipass if we can't communicate with the Flash
    if (flash_init_result != RETURN_OK) 
    {
        oledSetXY(2,0);
        printf_P(PSTR("Problem flash init"));
        while(1);
    } 
    
    // Check if we can initialize the touch sensing element
    touch_init_result = initTouchSensing();
    if (touch_init_result != RETURN_OK)
    {
        oledSetXY(2,0);
        printf_P(PSTR("Problem touch init"));
        delay_ms(2000);
    }
    
    // Launch the after touch init tests
    afterTouchInitTests();

    // write bitmap to inactive buffer and make the buffer 
    // active by scrolling it up.
    // Note: writing is automatically switch to inactive buffer
    oledWriteInactiveBuffer();
    oledBitmapDraw(0,0, &image_HaD_Mooltipass, OLED_SCROLL_UP);
    oledClear();    // clear inactive buffer
    
    // Light up the front panel
    //setPwmDc(0x0200);
    
//     uint16_t i;
//     while(1)
//     {
//         for (i = 0; i < 11; i++)
//         {
//             setPwmDc(1 << i);
//             delay_ms(500);
//         }
//         delay_ms(500);
//         for (i = 0; i < 11; i++)
//         {
//             setPwmDc(1 << (10-i));
//             delay_ms(500);
//         }           
//     }

    while (1)
    {
        card_detect_ret = isCardPlugged();
        if (card_detect_ret == RETURN_JDETECT)                          // Card just detected
        {
            temp_rettype = cardDetectedRoutine();
            
            #ifdef DEBUG_SMC_SCREEN_PRINT
                //oledFlipBuffers(OLED_SCROLL_DOWN,0);
                oledWriteInactiveBuffer();
                oledClear();
            #endif

            if (temp_rettype == RETURN_MOOLTIPASS_INVALID)              // Invalid card
            {
                _delay_ms(3000);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                    // Shut down card reader
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_PB)              // Problem with card
            {
                _delay_ms(3000);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                    // Shut down card reader
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_BLOCKED)         // Card blocked
            {
                _delay_ms(3000);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                    // Shut down card reader
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_BLANK)           // Blank Mooltipass card
            {   
                // Here we should ask the user to setup his mooltipass card and then call writeCodeProtectedZone() with 8 bytes
                _delay_ms(3000);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                     // Shut down card reader
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_USER)             // Configured mooltipass card
            {
                // Here we should ask the user for his pin and call mooltipassDetectedRoutine
                _delay_ms(3000);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                     // Shut down card reader
            }
        }
        else if (card_detect_ret == RETURN_JRELEASED)   //card just released
        {
            oledBitmapDraw(0,0, &image_HaD_Mooltipass, OLED_SCROLL_UP);
            removeFunctionSMC();
        }
    }
}
