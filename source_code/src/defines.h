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
/*  This project should be built differently
 *  depending on the Mooltipass version.
 *  Simply define one of these:
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
 *  => final version for the beta testers units
 *
 *  PREPRODUCTION_KICKSTARTER_SETUP
 *  => pre production run before mass production
 *
 *  PRODUCTION_KICKSTARTER_SETUP
 *  => final version for production for kickstarter units
 *
 *  PRODUCTION_TEST_SETUP
 *  => exactly like the kickstarter product setup, but with no check on fuses (used for isp programming & test)
 *
 *  MINI_CLICK_BETATESTERS_SETUP
 *  => mini beta testing units with click scroll wheel, sent to the beta testers
*/
#define PRODUCTION_TEST_SETUP
#if defined(BETATESTERS_SETUP)
    #define FLASH_CHIP_32M
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_OLIVIER_V1
    #define NO_PIN_CODE_REQUIRED
    #define AVR_BOOTLOADER_PROGRAMMING
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(BETATESTERS_SETUP_PIN)
    #define FLASH_CHIP_32M
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_OLIVIER_V1
    #define AVR_BOOTLOADER_PROGRAMMING
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(BETATESTERS_AUTOACCEPT_SETUP)
    #define FLASH_CHIP_32M
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_OLIVIER_V1
    #define ALWAYS_ACCEPT_REQUESTS
    #define AVR_BOOTLOADER_PROGRAMMING
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(PRODUCTION_SETUP)
    #define FLASH_CHIP_32M
    #define HARDWARE_OLIVIER_V1
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
    // TO REMOVE IN THE FUTURE!!! //
    #define AVR_BOOTLOADER_PROGRAMMING
#elif defined(PREPRODUCTION_KICKSTARTER_SETUP)
    #define FLASH_CHIP_4M
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_OLIVIER_V1
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
    // TO REMOVE IN THE FUTURE!!! //
    #define AVR_BOOTLOADER_PROGRAMMING
#elif defined(PRODUCTION_KICKSTARTER_SETUP)
    #define FLASH_CHIP_4M
    #define HARDWARE_OLIVIER_V1
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(PRODUCTION_TEST_SETUP)
    //#define STACK_DEBUG
    #define FLASH_CHIP_4M
    #define DEV_PLUGIN_COMMS
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_OLIVIER_V1
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
#elif defined(MINI_CLICK_BETATESTERS_SETUP)
    #define MINI_VERSION
    #define FLASH_CHIP_4M
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_MINI_CLICK_V1
    #define AVR_BOOTLOADER_PROGRAMMING
    #define ENABLE_MOOLTIPASS_CARD_FORMATTING
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
// Hardware test procedure
//#define HW_TEST_PROC

/**************** ENABLING TESTS ****************/
// As they may be manually enabled as well
#ifdef PRODUCTION_TEST_ENABLED
    #define TESTS_ENABLED
#endif

/**************** PRINTF ACTIVATION ****************/
#if defined(HW_TEST_PROC) || defined(PRODUCTION_TEST_ENABLED) || defined(DEBUG_SMC_SCREEN_PRINT) || defined(DEBUG_SMC_USB_PRINT) || defined(FLASH_TEST_DEBUG_OUTPUT_USB) || defined(GENERAL_LOGIC_OUTPUT_USB) || defined(CMD_PARSER_USB_DEBUG_OUTPUT) || defined(USB_DEBUG_OUTPUT)
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

/**************** VALUE DEFINE PRINTOUT ****************/
#define XSTR(x) STR(x)
#define STR(x) #x

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
//#define FLASH_BLOCK_IMPORT_EXPORT
#define NODE_BLOCK_IMPORT_EXPORT

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
enum new_pinreturn_type_t       {RETURN_NEW_PIN_NOK = -1, RETURN_NEW_PIN_OK = 0, RETURN_NEW_PIN_DIFF = 1};
enum usb_com_return_t           {RETURN_COM_NOK = -1, RETURN_COM_TRANSF_OK = 0, RETURN_COM_TIMEOUT = 1};
enum valid_card_det_return_t    {RETURN_VCARD_NOK = -1, RETURN_VCARD_OK = 0, RETURN_VCARD_UNKNOWN = 1};
enum detect_return_t            {RETURN_REL = 0, RETURN_DET, RETURN_JDETECT, RETURN_JRELEASED};
enum button_return_t            {LEFT_BUTTON = 0, RIGHT_BUTTON = 1, GUARD_BUTTON = 2};
enum service_compare_mode_t     {COMPARE_MODE_MATCH = 0, COMPARE_MODE_COMPARE = 1};
enum service_type_t             {SERVICE_CRED_TYPE = 0, SERVICE_DATA_TYPE = 1};
enum timer_flag_t               {TIMER_EXPIRED = 0, TIMER_RUNNING = 1};
enum return_type_t              {RETURN_NOK = -1, RETURN_OK = 0};
enum flash_ret_t                {RETURN_INVALID_PARAM = -2, RETURN_WRITE_ERR = -3, RETURN_READ_ERR = -4, RETURN_NO_MATCH = -5};
enum justify_t                  {OLED_LEFT  = 0, OLED_RIGHT = 1, OLED_CENTRE = 2};

/**************** TYPEDEFS ****************/
typedef void (*bootloader_f_ptr_type)(void);
typedef int8_t RET_TYPE;

/**************** VERSION DEFINES ***************/
#ifndef MOOLTIPASS_VERSION
    #define MOOLTIPASS_VERSION "v1.1"
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
#ifdef  HARDWARE_MINI_CLICK_V1
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
    #define PORTID_SC_DET   PORTC7
    #define PORT_SC_DET     PORTC
    #define DDR_SC_DET      DDRC
    #define PIN_SC_DET      PINC
    // Smart card program
    #define PORTID_SC_PGM   PORTB7
    #define PORT_SC_PGM     PORTB
    #define DDR_SC_PGM      DDRB
    // Smart card power enable
    #define PORTID_SC_POW   PORTB5
    #define PORT_SC_POW     PORTB
    #define DDR_SC_POW      DDRB
    // Smart card reset
    #define PORTID_SC_RST   PORTF0
    #define PORT_SC_RST     PORTF
    #define DDR_SC_RST      DDRF
    // OLED Data / Command
    #define PORTID_OLED_DnC PORTD4
    #define PORT_OLED_DnC   PORTD
    #define DDR_OLED_DnC    DDRD
    // OLED Slave Select
    #define PORTID_OLED_SS  PORTD7
    #define PORT_OLED_SS    PORTD
    #define DDR_OLED_SS     DDRD
    // OLED reset
    #define PORTID_OLED_nR  PORTD6
    #define PORT_OLED_nR    PORTD
    #define DDR_OLED_nR     DDRD
    // Power enable to the OLED
    #define PORTID_OLED_POW PORTE2
    #define PORT_OLED_POW   PORTE
    #define DDR_OLED_POW    DDRE
    // 5 direction joystick
    #define PORTID_JOY_UP       PORTF1
    #define PORTID_JOY_DOWN     PORTF4
    #define PORTID_JOY_LEFT     PORTF5
    #define PORTID_JOY_RIGHT    PORTF7
    #define PORTID_JOY_CENTER   PORTF6
    #define PORT_JOYSTICK       PORTF
    #define DDR_JOYSTICK        DDRF
    #define PIN_JOYSTICK        PINF
    // Click wheel
    #define PORTID_WHEEL_A      PORTC6
    #define PIN_WHEEL_A         PINC
    #define PORTID_WHEEL_B      PORTB6
    #define PIN_WHEEL_B         PINB
    #define PORT_WHEEL_A        PORTC
    #define PORT_WHEEL_B        PORTB
    #define DDR_WHEEL_A         DDRC
    #define DDR_WHEEL_B         DDRB
    #define PORTID_CLICK        PORTE6
    #define PORT_CLICK          PORTE
    #define DDR_CLICK           DDRE
    #define PIN_CLICK           PINE
#endif

#endif /* DEFINES_H_ */
