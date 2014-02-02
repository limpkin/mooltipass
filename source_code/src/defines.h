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

/*!	\file 	defines.h
*	\brief	Project definitions
*	Created: 08/12/2013 14:03:47
*	Author: Mathieu Stephan
*/


#ifndef DEFINES_H_
#define DEFINES_H_

#include <stdint.h>

/** HARDWARE VERSION **/
#define	HARDWARE_V1

/** CPU SPEED **/
//#define F_CPU 16000000	//in project definition?

/** MACROS **/
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

/** ENUMS **/
enum card_detect_return_t	{RETURN_CARD_NDET, RETURN_CARD_TEST_PB, RETURN_CARD_4_TRIES_LEFT,  RETURN_CARD_3_TRIES_LEFT,  RETURN_CARD_2_TRIES_LEFT,  RETURN_CARD_1_TRIES_LEFT, RETURN_CARD_0_TRIES_LEFT};
enum pin_check_return_t		{RETURN_PIN_OK, RETURN_PIN_NOK_3, RETURN_PIN_NOK_2, RETURN_PIN_NOK_1, RETURN_PIN_NOK_0};
enum detect_return_t		{RETURN_REL, RETURN_DET, RETURN_JDETECT, RETURN_JRELEASED,RETURN_NO_CHANGE};
enum return_type			{RETURN_NOK = 0, RETURN_OK, RETURN_NOT_INIT};
enum spi_type				{SPI_NATIVE, SPI_USART};

/** DEFINES FLASH **/
#define CREDENTIAL_BLOCK_SIZE		88
#define READY_FLASH_BITMASK			0x80
#define	OPCODE_MAN_DEV_ID_READ		0x9F
#define OPCODE_LOWF_READ			0x03
#define OPCODE_BUFFER_READ			0xD1
#define OPCODE_MAINP_TO_BUFFER		0x53
#define OPCODE_READ_STAT_REG		0xD7
#define OPCODE_MMP_PROG_TBUFFER		0x82

/** DEFINES SMART CARD **/
#define SMARTCARD_FABRICATION_ZONE	0x0F0F
#define SMARTCARD_FACTORY_PIN		0xF0F0
#define SMARTCARD_DEFAULT_PIN		0xDEAD

/** DEFINES FIRMWARE **/
#define FALSE			0
#define TRUE			(!FALSE)

/** DEFINES PORTS **/
#ifdef HARDWARE_V1
	// SPIs
	#define SPI_SMARTCARD	SPI_NATIVE
	#define	SPI_FLASH		SPI_USART
	#define SPI_OLED		SPI_USART
	// Slave Select Flash
	#define PORTID_FLASH_nS	PORTB4
	#define PORT_FLASH_nS	PORTB
	#define DDR_FLASH_nS	DDRB
	// Detect smart card
	#define PORTID_SC_DET	PORTB6
	#define PORT_SC_DET		PORTB
	#define DDR_SC_DET		DDRB
	#define PIN_SC_DET		PINB
	// Smart card program
	#define PORTID_SC_PGM	PORTC6
	#define PORT_SC_PGM		PORTC
	#define DDR_SC_PGM		DDRC
	// Smart card power enable
	#define PORTID_SC_POW	PORTE6
	#define PORT_SC_POW		PORTE
	#define DDR_SC_POW		DDRE
	// Smart card reset
	#define PORTID_SC_RST	PORTB5
	#define PORT_SC_RST		PORTB
	#define DDR_SC_RST		DDRB
	// OLED Data / Command
	#define PORTID_OLED_DnC	PORTD7
	#define PORT_OLED_DnC	PORTD
	#define DDR_OLED_DnC	DDRD
	// OLED Slave Select
	#define PORTID_OLED_SS	PORTD6
	#define PORT_OLED_SS	PORTD
	#define DDR_OLED_SS		DDRD
	// OLED reset
	#define PORTID_OLED_nR	PORTD1
	#define PORT_OLED_nR	PORTD
	#define DDR_OLED_nR		DDRD
	// Power enable to the OLED
	#define PORTID_OLED_POW	PORTB7
	#define PORT_OLED_POW	PORTB
	#define DDR_OLED_POW	DDRB
#endif

/** DEFINES HARDWARE **/
#define FLASH_MANUF_ID	0x1F
#define SMARTCARD_FZ	0x00

/** DEFINES OLED SCREEN **/
#define	OLED_Shift		0x1C
#define OLED_Max_Column	0x3F			// 256/4-1
#define OLED_Max_Row	0x3F			// 64-1
#define	OLED_Brightness	0x0A
#define OLED_Contrast	0x9F

/** BITMAP DEFINES **/
#define HACKADAY_BMP				0x00

/** TYPEDEFS **/
typedef uint8_t RET_TYPE;

#endif /* DEFINES_H_ */