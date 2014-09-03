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


/**************** SETUP DEFINES ****************/
// Please choose your sensitivity here
//#define LOW_SENSITIVITY
/*
 *  V1_DEVELOPERS_BOOTLOADER_SETUP
 *  => the first hardware version, with bootloader
 *  
 *  V1_DEVELOPERS_ISP_SETUP
 *  => the first hardware version, without bootloader
 *
 *  V2_DEVELOPERS_BOOTLOADER_SETUP
 *  => final hardware version for developpers, with bootloader
 *
 *  V2_DEVELOPERS_BOTPCB_BOOTLOADER_SETUP
 *  => same as above, but without top PCB
 *
 *  V2_DEVELOPERS_ISP_SETUP
 *  => final hardware version for developpers, without bootloader (for Mike)
 * 
 *  BETATESTERS_SETUP
 *  => version sent to the beta testers
 *
 *  BETATESTERS_SETUP_PIN
 *  => Same as above but with PIN
 *
 *  BETATESTERS_AUTOACCEPT_SETUP
 *  => Same as above, but always accepts requests
 *
 *  PRODUCTION_SETUP
 *  => final version for production
*/
#define BETATESTERS_SETUP_PIN
#if defined(V1_DEVELOPERS_BOOTLOADER_SETUP)
    #define STACK_DEBUG
    #define HARDWARE_V1
    #define TESTS_ENABLED
    #define FLASH_CHIP_1M
    #define DEV_PLUGIN_COMMS
    #define SMARTCARD_FUSE_V1
    #define NO_PIN_CODE_REQUIRED
    #define AVR_BOOTLOADER_PROGRAMMING
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(V1_DEVELOPERS_ISP_SETUP)
    #define STACK_DEBUG
    #define HARDWARE_V1
    #define TESTS_ENABLED
    #define FLASH_CHIP_1M
    #define DEV_PLUGIN_COMMS
    #define SMARTCARD_FUSE_V1
    #define NO_PIN_CODE_REQUIRED
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(V2_DEVELOPERS_BOOTLOADER_SETUP) || defined(V2_DEVELOPERS_BOTPCB_BOOTLOADER_SETUP)
    #define STACK_DEBUG
    #define TESTS_ENABLED
    #define FLASH_CHIP_1M
    #define DEV_PLUGIN_COMMS
    #define HARDWARE_OLIVIER_V1
    #define NO_PIN_CODE_REQUIRED
    #define AVR_BOOTLOADER_PROGRAMMING
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(V2_DEVELOPERS_ISP_SETUP)
    #define STACK_DEBUG
    #define TESTS_ENABLED
    #define FLASH_CHIP_1M
    #define DEV_PLUGIN_COMMS
    #define HARDWARE_OLIVIER_V1
    #define NO_PIN_CODE_REQUIRED
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(BETATESTERS_SETUP)
    #define FLASH_CHIP_32M
    #define DEV_PLUGIN_COMMS
    #define HARDWARE_OLIVIER_V1
    #define NO_PIN_CODE_REQUIRED
    #define AVR_BOOTLOADER_PROGRAMMING
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(BETATESTERS_SETUP_PIN)
    #define FLASH_CHIP_32M
    #define DEV_PLUGIN_COMMS
    #define HARDWARE_OLIVIER_V1
    #define AVR_BOOTLOADER_PROGRAMMING
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(BETATESTERS_AUTOACCEPT_SETUP)
    #define FLASH_CHIP_32M
    #define DEV_PLUGIN_COMMS
    #define HARDWARE_OLIVIER_V1
    #define ALWAYS_ACCEPT_REQUESTS
    #define AVR_BOOTLOADER_PROGRAMMING
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(PRODUCTION_SETUP)
    #define FLASH_CHIP_32M
    #define HARDWARE_OLIVIER_V1
    // TO REMOVE IN THE FUTURE!!! //
    #define AVR_BOOTLOADER_PROGRAMMING
#endif

/**************** DEBUG PRINTS ****************/
// Used for smart card testing
//#define DEBUG_SMC_DUMP_USB_PRINT
//#define DEBUG_SMC_USB_PRINT
// Used for flash testing prints
//#define FLASH_TEST_DEBUG_OUTPUT_USB
//#define FLASH_TEST_DEBUG_OUTPUT_OLED
// Used for touch interface
//#define TOUCH_DEBUG_OUTPUT_USB
// User for general logic
//#define GENERAL_LOGIC_OUTPUT_USB
// Used for parser general logic
//#define CMD_PARSER_USB_DEBUG_OUTPUT
// Used for USB communications
//#define USB_DEBUG_OUTPUT
// Used for production tests
//#define PRODUCTION_TEST_ENABLED

/**************** ENABLING TESTS ****************/
// As they may be manually enabled as well
#ifdef PRODUCTION_TEST_ENABLED
    #define TESTS_ENABLED
#endif

/**************** PRINTF ACTIVATION ****************/
#if defined(PRODUCTION_TEST_ENABLED) || defined(DEBUG_SMC_SCREEN_PRINT) || defined(DEBUG_SMC_USB_PRINT) || defined(FLASH_TEST_DEBUG_OUTPUT_USB) || defined(GENERAL_LOGIC_OUTPUT_USB) || defined(CMD_PARSER_USB_DEBUG_OUTPUT) || defined(USB_DEBUG_OUTPUT)
    #define ENABLE_PRINTF
#else
    #undef ENABLE_PRINTF
#endif

#ifndef ENABLE_PRINTF
    #define printf(...)
    #define printf_P(...)
    #define usbPrintf(...)
    #define usbPrintf_P(...)
#endif

/**************** HARDWARE VERSION ****************/
// First hardware sent to the contributors, 12/2013
//#define HARDWARE_V1
// Olivier's design hardware, 04/2014
//#define HARDWARE_OLIVIER_V1

/**************** PROGRAMMING HARDWARE ****************/
// Uncomment to go to the original boot loader when smart card inserted at boot
//#define AVR_BOOTLOADER_PROGRAMMING

/**************** SMARTCARD FUSE VERSION ****************/
// First smart card sent to the first contributors
//#define SMARTCARD_FUSE_V1

/**************** PIN HANDLING ******************/
// Comment to let the user enter his PIN
//#define NO_PIN_CODE_REQUIRED

/************** SMARTCARD FORMATING **************/
// Comment to prevent mooltipass card formatting (for production)
//#define ENABLE_MOOLTIPASS_CARD_FORMATTING

/************** MILLISECOND DEBUG TIMER ***************/
//#define ENABLE_MILLISECOND_DBG_TIMER

/************** LOW LEVEL MEMORY BOUNDARY CHECKS ***************/
#define MEMORY_BOUNDARY_CHECKS

/************** IMPORT/EXPORT MODE FOR PLUGIN COMMS ***************/
#define FLASH_BLOCK_IMPORT_EXPORT
//#define NODE_BLOCK_IMPORT_EXPORT

/************** TESTS ENABLING ***************/
// Comment to disable test calls
//#define TESTS_ENABLED

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
enum mooltipass_detect_return_t {RETURN_MOOLTIPASS_INVALID = 0, RETURN_MOOLTIPASS_PB = 1, RETURN_MOOLTIPASS_BLOCKED = 2, RETURN_MOOLTIPASS_BLANK = 3, RETURN_MOOLTIPASS_USER = 4, RETURN_MOOLTIPASS_0_TRIES_LEFT = 5, RETURN_MOOLTIPASS_1_TRIES_LEFT = 6, RETURN_MOOLTIPASS_2_TRIES_LEFT = 7, RETURN_MOOLTIPASS_3_TRIES_LEFT = 8, RETURN_MOOLTIPASS_4_TRIES_LEFT = 9};
enum touch_detect_return_t      {RETURN_NO_CHANGE = 0x00, RETURN_LEFT_PRESSED = 0x01, RETURN_LEFT_RELEASED = 0x02, RETURN_RIGHT_PRESSED = 0x04, RETURN_RIGHT_RELEASED = 0x08, RETURN_WHEEL_PRESSED = 0x10, RETURN_WHEEL_RELEASED = 0x20, RETURN_PROX_DETECTION = 0x40, RETURN_PROX_RELEASED = 0x80};
enum card_detect_return_t       {RETURN_CARD_NDET, RETURN_CARD_TEST_PB, RETURN_CARD_4_TRIES_LEFT,  RETURN_CARD_3_TRIES_LEFT,  RETURN_CARD_2_TRIES_LEFT,  RETURN_CARD_1_TRIES_LEFT, RETURN_CARD_0_TRIES_LEFT};
enum pin_check_return_t         {RETURN_PIN_OK = 0, RETURN_PIN_NOK_3, RETURN_PIN_NOK_2, RETURN_PIN_NOK_1, RETURN_PIN_NOK_0};
enum pass_check_return_t        {RETURN_PASS_CHECK_NOK = -1, RETURN_PASS_CHECK_OK = 0, RETURN_PASS_CHECK_BLOCKED = 1};
enum usb_com_return_t           {RETURN_COM_NOK = -1, RETURN_COM_TRANSF_OK = 0, RETURN_COM_TIMEOUT = 1};
enum detect_return_t            {RETURN_REL = 0, RETURN_DET, RETURN_JDETECT, RETURN_JRELEASED};
enum button_return_t            {LEFT_BUTTON = 0, RIGHT_BUTTON = 1, GUARD_BUTTON = 2};
enum timer_flag_t               {TIMER_EXPIRED = 0, TIMER_RUNNING = 1};
enum return_type_t              {RETURN_NOK = -1, RETURN_OK = 0};
enum flash_ret_t                {RETURN_INVALID_PARAM = -2, RETURN_WRITE_ERR = -3, RETURN_READ_ERR = -4, RETURN_NO_MATCH = -5};

/**************** TYPEDEFS ****************/
typedef void (*bootloader_f_ptr_type)(void);
typedef int8_t RET_TYPE;

/**************** VERSION DEFINES ***************/
#ifndef MOOLTIPASS_VERSION
    #define MOOLTIPASS_VERSION "unknown"
#endif

/**************** FLASH TEST SELECTION ****************/
#define RUN_FLASH_TEST_WR
#define RUN_FLASH_TEST_WRO
#define RUN_FLASH_TEST_ERASE_PAGE
#define RUN_FLASH_TEST_ERASE_BLOCK
#define RUN_FLASH_TEST_ERASE_SECTOR_X
#define RUN_FLASH_TEST_ERASE_SECTOR_0

/**************** FEATURE SELECTION ****************/
// Used for normal browser plugin communications
#define USB_FEATURE_PLUGIN_COMMS            

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
