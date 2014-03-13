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
*   \brief  Main file
*   Created: 08/12/2013 13:54:34
*   Author: Mathieu Stephan
*/
#include "smart_card_higher_level_functions.h"
#include "aes256_nessie_test.h"
#include "aes256_ctr_test.h"
#include "usb_serial_hid.h"
#include "mooltipass.h"
#include "interrupts.h"
#include "smartcard.h"
#include "flash_mem.h"
#include "oledmp.h"
#include "spi.h"
#include <util/delay.h>
#include <stdlib.h>
#include <avr/io.h>

#include "had_mooltipass.h"


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

// Perhaps move this function in another file later?
uint16_t mooltipass_rand(void)
{
    return (uint16_t)rand();
}

/*! \fn     main(void)
*   \brief  Main function
*/
int main(void)
{
    RET_TYPE flash_init_result = RETURN_NOK;
    RET_TYPE card_detect_ret;
    RET_TYPE temp_rettype;

    CPU_PRESCALE(0);                    // Set for 16MHz clock
    _delay_ms(500);                     // Let the power settle
    initPortSMC();                      // Initialize smart card Port
    initIRQ();                          // Initialize interrupts
    usb_init();                         // Initialize USB controller

    spiUsartBegin(SPI_RATE_8_MHZ);
    while(!usb_configured());           // Wait for host to set configuration

    // set up OLED now that USB is receiving full 500mA.
    oledBegin(FONT_DEFAULT);
    oledSetColour(15);
    oledSetBackground(0);
    oledSetContrast(OLED_Contrast);

    flash_init_result = initFlash();    // Initialize flash memory

    //#define TEST_HID_AND_CDC
    #ifdef TEST_HID_AND_CDC
        //Show_String("Z",FALSE,2,0);
        //usb_keyboard_press(KEY_S, 0);
        while(1)
        {
            int n = usb_serial_getchar();
            if (n >= 0)
            {
                usb_serial_putchar(n);
                oledSetXY(2,0);
                ole_putch((char)n);

                //usb_keyboard_press(n,0);
            }

        }
    #endif /* TEST_HID_AND_CDC */

    //#define NESSIE_TEST_VECTORS
    #ifdef NESSIE_TEST_VECTORS
        while(1)
        {
            // msg into oled display
            oledSetXY(2,0);
            oledPutstr_P(PSTR("send s to start nessie test"));

            int input0 = usb_serial_getchar();

            nessieOutput = &usb_serial_putchar;

            // do nessie test after sending s or S chars
            if (input0 == 's' || input0 == 'S')
            {
                nessieTest(1);
                nessieTest(2);
                nessieTest(3);
                nessieTest(4);
                nessieTest(5);
                nessieTest(6);
                nessieTest(7);
                nessieTest(8);
            }
        }
    #endif
    
    //#define CTR_TEST_VECTORS
    #ifdef CTR_TEST_VECTORS
        while(1)
        {
            // msg into oled display
            oledSetXY(2,0);
            oledPutstr_P(PSTR("send s to start CTR test"));

            int input1 = usb_serial_getchar();

            ctrTestOutput = &usb_serial_putchar;

            // do ctr test after sending s or S chars
            if (input1 == 's' || input1 == 'S')
            {
                aes256CtrTest();
            }
        }
    #endif

    oledSetXY(2,0);
    if (flash_init_result == RETURN_OK) 
    {
        oledPutstr_P(PSTR("Flash init ok"));
    } 
    else 
    {
        oledPutstr_P(PSTR("Problem flash init"));
    }
    _delay_ms(1000);
    oledClear();
    oledBitmapDraw(0,0, &image_HaD_Mooltipass);

    while (1)
    {
        card_detect_ret = isCardPlugged();
        if (card_detect_ret == RETURN_JDETECT)                           // Card just detected
        {
            temp_rettype = cardDetectedRoutine();

            if (temp_rettype == RETURN_MOOLTIPASS_INVALID)               // Invalid card
            {
                _delay_ms(3000);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                    // Shut down card reader
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_PB)               // Problem with card
            {
                _delay_ms(3000);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                    // Shut down card reader
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_BLOCKED)          // Card blocked
            {
                _delay_ms(3000);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                    // Shut down card reader
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_BLANK)            // Blank mooltipass card
            {
                // Here we should ask the user to setup his mooltipass card
                _delay_ms(3000);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                    // Shut down card reader
            }
            else if (temp_rettype == RETURN_MOOLTIPASS_USER)             // Configured mooltipass card
            {
                // Here we should ask the user for his pin and call mooltipassdetect
                _delay_ms(3000);
                printSMCDebugInfoToScreen();
                removeFunctionSMC();                                    // Shut down card reader
            }
            /*read_credential_block_within_flash_page(2,1,temp_buffer);
            for(i = 0; i < 10; i++)
            {
                hexachar_to_string(temp_buffer[i], temp_string);
                Show_String(temp_string, FALSE, 2+i*5, 0);
            }
            temp_buffer[3] = 0x0A;
            write_credential_block_within_flash_page(2,1, temp_buffer);
            read_credential_block_within_flash_page(2,1,temp_buffer);
            for(i = 0; i < 10; i++)
            {
                hexachar_to_string(temp_buffer[i], temp_string);
                Show_String(temp_string, FALSE, 2+i*5, 8);
            }*/
        }
        else if (card_detect_ret == RETURN_JRELEASED)   //card just released
        {
            removeFunctionSMC();
            oledClear();
            oledBitmapDraw(0,0, &image_HaD_Mooltipass);
        }
    }
}
