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

// Perhaps move this function in another file later?
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
	RET_TYPE card_detect_ret;
	RET_TYPE temp_rettype;

	uint8_t temp_buffer[200];
	char temp_string[20];

	char james_usb_test[20];

	CPU_PRESCALE(0);					// Set for 16MHz clock
	_delay_ms(500);						// Let the power settle
	initPortSMC();						// Init smart card Port
	initIRQ();							// Init interrupts	
	usb_init();							// Init USB controller
	while(!usb_configured());			// Wait for host to set configuration	
	initOLED();							// Init OLED screen after enum
	flash_init_result = initFlash();	// Init flash memory

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
			temp_rettype = cardDetectedRoutine();
			
			if(temp_rettype == RETURN_MOOLTIPASS_INVALID)				// Invalid card
			{
				_delay_ms(3000);
				printSMCDebugInfoToScreen();
				removeFunctionSMC();									// Shut down card reader
			} 
			else if(temp_rettype == RETURN_MOOLTIPASS_PB)				// Problem with card
			{
				_delay_ms(3000);
				printSMCDebugInfoToScreen();
				removeFunctionSMC();									// Shut down card reader
			}
			else if(temp_rettype == RETURN_MOOLTIPASS_BLOCKED)			// Card blocked
			{
				_delay_ms(3000);
				printSMCDebugInfoToScreen();
				removeFunctionSMC();									// Shut down card reader
			}
			else if(temp_rettype == RETURN_MOOLTIPASS_BLANK)			// Blank mooltipass card
			{
				// Here we should ask the user to setup his mooltipass card
				_delay_ms(3000);
				printSMCDebugInfoToScreen();
				removeFunctionSMC();									// Shut down card reader
			}
			else if(temp_rettype == RETURN_MOOLTIPASS_USER)				// Configured mooltipass card
			{
				// Here we should ask the user for his pin and call mooltipassdetect
				_delay_ms(3000);
				printSMCDebugInfoToScreen();
				removeFunctionSMC();									// Shut down card reader
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
		else if(card_detect_ret == RETURN_JRELEASED)	//card just released
		{
			removeFunctionSMC();
			clear_screen();
			Show_String("Please insert card", FALSE, 2, 8);
		}
    }
}