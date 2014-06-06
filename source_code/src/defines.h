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
/* \file    defines.h
 * \brief   Project definitions
 *  Created: 11/01/2014 11:54:26
 *  Author: Mathieu Stephan
 */
#ifndef DEFINES_H_
#define DEFINES_H_

#include <avr/io.h>
#include <stdint.h>

/**************** DEBUG PRINTFS ****************/
// Used for smart card testing
#define DEBUG_SMC_SCREEN_PRINT
#define DEBUG_SMC_USB_PRINT
// Used for flash testing prints
#define FLASH_TEST_DEBUG_OUTPUT_USB
#define FLASH_TEST_DEBUG_OUTPUT_OLED
// Used for touch interface
#define TOUCH_DEBUG_OUTPUT_USB

/**************** HARDWARE VERSION ****************/
// First hardware sent to the contributors, 12/2013
//#define HARDWARE_V1
// Olivier's design hardware, 04/2014
#define HARDWARE_OLIVIER_V1

/**************** PROGRAMMING HARDWARE ****************/
// Uncomment to go to the original boot loader when smart card inserted at boot
//#define AVR_BOOTLOADER_PROGRAMMING

/**************** SMARTCARD FUSE VERSION ****************/
// First smart card sent to the first contributors
#define SMARTCARD_FUSE_V1

/**************** HW MACROS ****************/
#define CPU_PRESCALE(n)         (CLKPR = 0x80, CLKPR = (n))

/**************** DEFINES FIRMWARE ****************/
#define AES_KEY_LENGTH          256
#define FALSE                   0
#define TRUE                    (!FALSE)

/**************** ASM "ENUMS" ****************/
#define SPI_NATIVE              1
#define SPI_USART               2

/**************** C ENUMS ****************/
enum mooltipass_detect_return_t {RETURN_MOOLTIPASS_INVALID, RETURN_MOOLTIPASS_PB, RETURN_MOOLTIPASS_BLOCKED, RETURN_MOOLTIPASS_BLANK, RETURN_MOOLTIPASS_USER, RETURN_MOOLTIPASS_4_TRIES_LEFT,  RETURN_MOOLTIPASS_3_TRIES_LEFT,  RETURN_MOOLTIPASS_2_TRIES_LEFT,  RETURN_MOOLTIPASS_1_TRIES_LEFT, RETURN_MOOLTIPASS_0_TRIES_LEFT};
enum touch_detect_return_t      {RETURN_NO_CHANGE = 0x00, RETURN_LEFT_PRESSED = 0x01, RETURN_LEFT_RELEASED = 0x02, RETURN_RIGHT_PRESSED = 0x04, RETURN_RIGHT_RELEASED = 0x08, RETURN_WHEEL_PRESSED = 0x10, RETURN_WHEEL_RELEASED = 0x20, RETURN_PROX_DETECTION = 0x40, RETURN_PROX_RELEASED = 0x80};
enum card_detect_return_t       {RETURN_CARD_NDET, RETURN_CARD_TEST_PB, RETURN_CARD_4_TRIES_LEFT,  RETURN_CARD_3_TRIES_LEFT,  RETURN_CARD_2_TRIES_LEFT,  RETURN_CARD_1_TRIES_LEFT, RETURN_CARD_0_TRIES_LEFT};
enum pin_check_return_t         {RETURN_PIN_OK = 0, RETURN_PIN_NOK_3, RETURN_PIN_NOK_2, RETURN_PIN_NOK_1, RETURN_PIN_NOK_0};
enum usb_com_return_t           {RETURN_COM_NOK = -1, RETURN_COM_TRANSF_OK = 0, RETURN_COM_TIMEOUT = 1};
enum detect_return_t            {RETURN_REL, RETURN_DET, RETURN_JDETECT, RETURN_JRELEASED};
enum button_return_t            {LEFT_BUTTON = 0, RIGHT_BUTTON = 1, GUARD_BUTTON = 2};
enum return_type_t              {RETURN_NOK = -1, RETURN_OK = 0};
enum flash_ret_t                {RETURN_INVALID_PARAM = -2, RETURN_WRITE_ERR = -3, RETURN_READ_ERR = -4, RETURN_NO_MATCH = -5};

/**************** TYPEDEFS ****************/
typedef void (*bootloader_f_ptr_type)(void);
typedef int8_t RET_TYPE;

/**************** BITMAP DEFINES ****************/
#define HACKADAY_BMP        0x00

/**************** FLASH TEST SELECTION ****************/
#define RUN_FLASH_TEST_WR
#define RUN_FLASH_TEST_WRO
#define RUN_FLASH_TEST_ERASE_PAGE
#define RUN_FLASH_TEST_ERASE_BLOCK
#define RUN_FLASH_TEST_ERASE_SECTOR_X
#define RUN_FLASH_TEST_ERASE_SECTOR_0

/**************** DEFINES PORTS ****************/
#ifdef HARDWARE_V1
    // SPIs
    #define SPI_SMARTCARD   SPI_NATIVE
    #define SPI_FLASH       SPI_USART
    #define SPI_OLED        SPI_USART
    #define DDR_SPI_NATIVE  DDRB
    #define PORT_SPI_NATIVE PORTB
    #define SS_SPI_NATIVE   PORTB0
    #define SCK_SPI_NATIVE  PORTB1
    #define MOSI_SPI_NATIVE PORTB2
    #define MISO_SPI_NATIVE PORTB3
    #define DDR_SPI_USART   DDRD
    #define PORT_SPI_USART  PORTD
    #define SCK_SPI_USART   PORTD5
    #define MOSI_SPI_USART  PORTD3
    #define MISO_SPI_USART  PORTD2
    // Slave Select Flash
    #define PORTID_FLASH_nS PORTB4
    #define PORT_FLASH_nS   PORTB
    #define DDR_FLASH_nS    DDRB
    // Detect smart card
    #define PORTID_SC_DET   PORTB6
    #define PORT_SC_DET     PORTB
    #define DDR_SC_DET      DDRB
    #define PIN_SC_DET      PINB
    // Smart card program
    #define PORTID_SC_PGM   PORTC6
    #define PORT_SC_PGM     PORTC
    #define DDR_SC_PGM      DDRC
    // Smart card power enable
    #define PORTID_SC_POW   PORTE6
    #define PORT_SC_POW     PORTE
    #define DDR_SC_POW      DDRE
    // Smart card reset
    #define PORTID_SC_RST   PORTB5
    #define PORT_SC_RST     PORTB
    #define DDR_SC_RST      DDRB
    // OLED Data / Command
    #define PORTID_OLED_DnC PORTD7
    #define PORT_OLED_DnC   PORTD
    #define DDR_OLED_DnC    DDRD
    // OLED Slave Select
    #define PORTID_OLED_SS  PORTD6
    #define PORT_OLED_SS    PORTD
    #define DDR_OLED_SS     DDRD
    // OLED reset
    #define PORTID_OLED_nR  PORTD1
    #define PORT_OLED_nR    PORTD
    #define DDR_OLED_nR     DDRD
    // Power enable to the OLED
    #define PORTID_OLED_POW PORTB7
    #define PORT_OLED_POW   PORTB
    #define DDR_OLED_POW    DDRB
    // Touch sensing change (dummy one)
    #define PORTID_TOUCH_C  PORTF6
    #define PORT_TOUCH_C    PORTF
    #define DDR_TOUCH_C     DDRF
#endif
#ifdef  HARDWARE_OLIVIER_V1
    // SPIs
    #define SPI_SMARTCARD   SPI_NATIVE
    #define SPI_FLASH       SPI_USART
    #define SPI_OLED        SPI_USART
    #define DDR_SPI_NATIVE  DDRB
    #define PORT_SPI_NATIVE PORTB
    #define SS_SPI_NATIVE   PORTB0
    #define SCK_SPI_NATIVE  PORTB1
    #define MOSI_SPI_NATIVE PORTB2
    #define MISO_SPI_NATIVE PORTB3
    #define DDR_SPI_USART   DDRD
    #define PORT_SPI_USART  PORTD
    #define SCK_SPI_USART   PORTD5
    #define MOSI_SPI_USART  PORTD3
    #define MISO_SPI_USART  PORTD2
    // Slave Select Flash
    #define PORTID_FLASH_nS PORTB7
    #define PORT_FLASH_nS   PORTB
    #define DDR_FLASH_nS    DDRB
    // Detect smart card
    #define PORTID_SC_DET   PORTF5
    #define PORT_SC_DET     PORTF
    #define DDR_SC_DET      DDRF
    #define PIN_SC_DET      PINF
    // Smart card program
    #define PORTID_SC_PGM   PORTC6
    #define PORT_SC_PGM     PORTC
    #define DDR_SC_PGM      DDRC
    // Smart card power enable
    #define PORTID_SC_POW   PORTB4
    #define PORT_SC_POW     PORTB
    #define DDR_SC_POW      DDRB
    // Smart card reset
    #define PORTID_SC_RST   PORTE6
    #define PORT_SC_RST     PORTE
    #define DDR_SC_RST      DDRE
    // OLED Data / Command
    #define PORTID_OLED_DnC PORTD7
    #define PORT_OLED_DnC   PORTD
    #define DDR_OLED_DnC    DDRD
    // OLED Slave Select
    #define PORTID_OLED_SS  PORTD6
    #define PORT_OLED_SS    PORTD
    #define DDR_OLED_SS     DDRD
    // OLED reset
    #define PORTID_OLED_nR  PORTD4
    #define PORT_OLED_nR    PORTD
    #define DDR_OLED_nR     DDRD
    // Power enable to the OLED
    #define PORTID_OLED_POW PORTE2
    #define PORT_OLED_POW   PORTE
    #define DDR_OLED_POW    DDRE
    // LED PWM
    #define PORTID_LED_PWM  PORTC7
    #define PORT_LED_PWM    PORTC
    #define DDR_LED_PWM     DDRC
    // I2C IOs
    #define PORTID_I2C_SCL  PORTD0
    #define PORT_I2C_SCL    PORTD
    #define DDR_I2C_SCL     DDRD
    #define PORTID_I2C_SDA  PORTD1
    #define PORT_I2C_SDA    PORTD
    #define DDR_I2C_SDA     DDRD
    // Touch sensing change
    #define PORTID_TOUCH_C  PORTF4
    #define PORT_TOUCH_C    PORTF
    #define DDR_TOUCH_C     DDRF
    #define PIN_TOUCH_C     PINF
#endif

#endif /* DEFINES_H_ */
