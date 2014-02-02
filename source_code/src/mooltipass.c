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
/*!	\file 	mooltipass.c
*	\brief	Main file
*	Created: 08/12/2013 13:54:34
*	Author: Mathieu Stephan
*/
#include "mooltipass.h"
#include <util/delay.h>
#include <stdlib.h>
#include <avr/io.h>
#include <stdio.h>


/*!	\fn 	disable_jtag(void)
*	\brief	Disable the JTAG module
*/
void disable_jtag(void)
{
	unsigned char temp;

	temp = MCUCR;
	temp |= (1<<JTD);
	MCUCR = temp;
	MCUCR = temp;
}

uint16_t mooltipass_rand(void)
{
	return (uint16_t)rand();
}

/*!	\fn 	main(void)
*	\brief	Main function
*/
int main(void)
{

	RET_TYPE flash_init_result = RETURN_NOK;
	RET_TYPE card_detection_result;
	RET_TYPE card_detect_ret;
	RET_TYPE temp_rettype;

	uint8_t temp_buffer[200];
	char temp_string[20];

	char james_usb_test[20];

	CPU_PRESCALE(0);					// Set for 16MHz clock
	_delay_ms(500);						// Let the power settle
	initPortSMC();						// Init smart card Port
	initOLED();							// Init OLED screen
	initIRQ();							// Init interrupts	
	flash_init_result = initFlash();	// Init flash memory
	usb_init();							// Init USB controller
	while(!usb_configured());			// Wait for host to set configuration

#ifdef TEST_HID_AND_CDC
	Show_String("Z",FALSE,2,0);
	usb_keyboard_press(KEY_S, 0);
	while(1)
	{
			int n = usb_serial_getchar();
			if (n >= 0) {usb_serial_putchar(n);
				//for (int i=18;i>0;i--){james[i]=james[i-1];}
			sprintf(james_usb_test,"%c",n);	
			Show_String(james_usb_test,FALSE,2,0);
			usb_keyboard_press((n%25)+4,0);
			}

	}
#endif /* TEST_HID_AND_CDC */

	//lcd_display_grayscale();
	if(flash_init_result == RETURN_OK)
		Show_String("Flash init ok", FALSE, 2, 0);
	else
		Show_String("Problem flash init", FALSE, 2, 250);

	//display_picture(HACKADAY_BMP, 20, 0);
	//draw_screen_frame();
	//Show_String("Mooltipass", FALSE, 32, 10);
	Show_String("No card detected", FALSE, 25, 45);	//If no card is inserted at boot time

    while(1)
    {
		card_detect_ret = isCardPlugged();
		if(card_detect_ret == RETURN_JDETECT)							// Card just detected
		{
			clear_screen();												// Clear screen before writing anything new
			card_detection_result = detectFunctionSMC();				// Get card detection result

			if(card_detection_result == RETURN_CARD_NDET)				// This is not a card or card is really broken!
			{
				Show_String("Not a card", FALSE, 2, 8);
				removeFunctionSMC();									// Shut down card reader
			}
			else if(card_detection_result == RETURN_CARD_TEST_PB)		// Card test problem
			{
				Show_String("Card test problem", FALSE, 2, 8);
				removeFunctionSMC();
			}
			else if(card_detection_result == RETURN_CARD_0_TRIES_LEFT)	// Card blocked
			{
				Show_String("Card blocked", FALSE, 2, 8);
				removeFunctionSMC();
			}
			else														// Card is good! do stuff!
			{
				// Detect if the card is blank by checking that the manufacturer zone is different from FFFF
				if(swap16(*(uint16_t*)readManufacturerZone(temp_buffer)) == 0xFFFF)
				{
					// Card is new - transform into mooltipass
					Show_String("Blank card, transforming...", FALSE, 2, 8);

					temp_rettype = securityValidationSMC(SMARTCARD_FACTORY_PIN);	// Try to authenticate with factory pin

					if(temp_rettype == RETURN_PIN_OK)								// Card is unlocked - transform
					{
						if(transformBlankCardIntoMooltipass() == RETURN_OK)
							Show_String("Card transformed!", FALSE, 2, 16);
						else
							Show_String("Couldn't transform card!", FALSE, 2, 16);
						_delay_ms(2000); printSMCDebugInfoToScreen();				// Show debug info 
					}
					else															// Card unlock failed. Show number of tries left
					{
						int_to_string(getNumberOfSecurityCodeTriesLeft(), temp_string);
						Show_String(temp_string, FALSE, 2, 16);
						Show_String("tries left, wrong pin", FALSE, 6, 16);
					}
				}
				else																// Card is already converted into a mooltipass
				{
					Show_String("Mooltipass card detected", FALSE, 2, 8);

					temp_rettype = securityValidationSMC(SMARTCARD_FACTORY_PIN);	// Try to unlock 

					if(temp_rettype == RETURN_PIN_OK)								// Unlock successful 
					{
						// Check that the card is in security mode 2 by reading the SC
						if(swap16(*(uint16_t*)readSecurityCode(temp_buffer)) != 0xFFFF)	// Card is in mode 1 - transform again
						{
							Show_String("Transforming...", FALSE, 2, 16);
							transformBlankCardIntoMooltipass();
							_delay_ms(4000);printSMCDebugInfoToScreen();
						}
						else																// Everything is in order - proceed
						{
							Show_String("PIN code checked !", FALSE, 2, 16);
							temp_buffer[0] = 0x80;
							temp_buffer[1] = 0x00;
							eraseApplicationZone1NZone2SMC(FALSE);
							writeSMC(736, 16, temp_buffer);
							eraseApplicationZone1NZone2SMC(TRUE);
							writeSMC(176, 16, temp_buffer);
							_delay_ms(2000);printSMCDebugInfoToScreen();							
						}						
					}
					else //Unlock failed
					{
						int_to_string(getNumberOfSecurityCodeTriesLeft(), temp_string);
						Show_String(temp_string, FALSE, 2, 16);
						Show_String("tries left, wrong pin", FALSE, 6, 16);
						_delay_ms(2000);printSMCDebugInfoToScreen();
					}
				}
			}
			//printSMCDebugInfoToScreen();
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
		else if(card_detect_ret == RETURN_JRELEASED)	//card just released
		{
			removeFunctionSMC();
			clear_screen();
			Show_String("Please insert card", FALSE, 2, 8);
		}
    }
}