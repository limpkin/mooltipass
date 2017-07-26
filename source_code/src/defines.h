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
 *  POST_KICKSTARTER_UPDATE_SETUP
 *  => firmware to flash to the kickstarter units to update them, doesn't contain functional & electrical tests
 *
 *  MINI_AVRISP_PROG_TEST_SETUP
 *  => mooltipass mini 4Mb programmed using an avrisp, no security, no check on fuses
 *
 *  MINI_CLICK_BETATESTERS_SETUP
 *  => mini beta testing units with click scroll wheel, sent to the beta testers
 *
 *  MINI_PREPRODUCTION_SETUP_ACC
 *  => mooltipass mini pre-production units, with or without accelerometer
 *
 *  MINI_PREPROD_KICKSTARTER_SETUP
 *  => mooltipass mini pre production kickstarter version (4Mb instead of the 8Mb)
 *
 *  MINI_KICKSTARTER_SETUP
 *  => mooltipass mini production kickstarter version (8Mb)
*/
#define MINI_KICKSTARTER_SETUP
//#define MINI_PREPRODUCTION_SETUP_ACC
//#define POST_KICKSTARTER_UPDATE_SETUP

#if defined(BETATESTERS_SETUP)
    #define FLASH_CHIP_4M
    #define TWO_CAPS_TRICK
    #define DATA_STORAGE_EN
	//#define FLASH_CHIP_32M
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_OLIVIER_V1
    #define NO_PIN_CODE_REQUIRED
    #define AVR_BOOTLOADER_PROGRAMMING
#elif defined(BETATESTERS_SETUP_PIN)
    #define FLASH_CHIP_32M
    #define TWO_CAPS_TRICK
    #define DATA_STORAGE_EN
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_OLIVIER_V1
    #define AVR_BOOTLOADER_PROGRAMMING
#elif defined(BETATESTERS_AUTOACCEPT_SETUP)
    #define FLASH_CHIP_32M
    #define TWO_CAPS_TRICK
    #define DATA_STORAGE_EN
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_OLIVIER_V1
    #define ALWAYS_ACCEPT_REQUESTS
    #define AVR_BOOTLOADER_PROGRAMMING
#elif defined(PRODUCTION_SETUP)
    #define FLASH_CHIP_32M
    #define TWO_CAPS_TRICK
    #define DATA_STORAGE_EN
    #define HARDWARE_OLIVIER_V1
    // TO REMOVE IN THE FUTURE!!! //
    #define AVR_BOOTLOADER_PROGRAMMING
#elif defined(PREPRODUCTION_KICKSTARTER_SETUP)
    #define FLASH_CHIP_4M
    #define TWO_CAPS_TRICK
    #define DATA_STORAGE_EN
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_OLIVIER_V1
    // TO REMOVE IN THE FUTURE!!! //
    #define AVR_BOOTLOADER_PROGRAMMING
#elif defined(PRODUCTION_KICKSTARTER_SETUP)
    #define FLASH_CHIP_4M
    #define TWO_CAPS_TRICK
    #define DATA_STORAGE_EN
    #define HARDWARE_OLIVIER_V1
#elif defined(POST_KICKSTARTER_UPDATE_SETUP)
    #define FLASH_CHIP_4M
    #define TWO_CAPS_TRICK
    #define DATA_STORAGE_EN
    #define HARDWARE_OLIVIER_V1
#elif defined(PRODUCTION_TEST_SETUP)
    //#define STACK_DEBUG
    #define FLASH_CHIP_4M
    #define TWO_CAPS_TRICK
    #define DATA_STORAGE_EN
    #define DEV_PLUGIN_COMMS
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_OLIVIER_V1
#elif defined(MINI_AVRISP_PROG_TEST_SETUP)
    #define STACK_DEBUG
    #define MINI_VERSION
    #define SKIP_TUTORIAL
    #define FLASH_CHIP_4M
    #define TWO_CAPS_TRICK
    //#define DATA_STORAGE_EN
    #define DEV_PLUGIN_COMMS
    #define HARDWARE_MINI_CLICK_V2
    #define DISABLE_FUNCTIONAL_TEST
    #define AVR_BOOTLOADER_PROGRAMMING
#elif defined(MINI_CLICK_BETATESTERS_SETUP)
    #define STACK_DEBUG
    #define MINI_VERSION
    #define FLASH_CHIP_4M
    #define TWO_CAPS_TRICK
    #define DATA_STORAGE_EN
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_MINI_CLICK_V1
    #define AVR_BOOTLOADER_PROGRAMMING
#elif defined(MINI_CREDENTIAL_MANAGEMENT)
    #define MINI_CLICK_BETATESTERS_SETUP
//    #define STACK_DEBUG
    #define MINI_VERSION
    #define FLASH_CHIP_4M
//    #define TWO_CAPS_TRICK
//    #define DATA_STORAGE_EN
    #define JTAG_FUSE_ENABLED
    #define HARDWARE_MINI_CLICK_V1
    #define DISABLE_FUNCTIONAL_TEST
    #define AVR_BOOTLOADER_PROGRAMMING
    #define ENABLE_CREDENTIAL_MANAGEMENT                    // WARNING: requires a new resource bundle.img with additional strings
    #define DISABLE_SINGLE_CREDENTIAL_ON_CARD_STORAGE
    #define REPLACE_FAVORITES_WITH_CREDENTIAL_MANAGEMENT    // replaces favorites selection menu with creds management menu
#elif defined(MINI_PREPRODUCTION_SETUP_ACC)
    #define STACK_DEBUG
    #define MINI_VERSION
    #define FLASH_CHIP_4M
    #define DATA_STORAGE_EN
    //#define DEV_PLUGIN_COMMS
    #define HARDWARE_MINI_CLICK_V2
    #define DISABLE_USB_SET_UID_DEV_PASSWORD_COMMANDS       // Comment if you were to make your own mooltipass
#elif defined(MINI_PREPROD_KICKSTARTER_SETUP)
    //#define STACK_DEBUG
    #define MINI_VERSION
    #define FLASH_CHIP_4M
    #define DATA_STORAGE_EN
    #define HARDWARE_MINI_CLICK_V2
    #define DISABLE_USB_SET_UID_DEV_PASSWORD_COMMANDS
    #define KNOCK_SETTINGS_CHANGE_PREVENT_WHEN_CARD_INSERTED
#elif defined(MINI_PREPROD_KICKSTARTER_SETUP_HARDENED_CREDENTIAL_MANAGEMENT)
    //#define STACK_DEBUG
    #define MINI_VERSION
    #define FLASH_CHIP_4M
    //#define DATA_STORAGE_EN
    #define HARDWARE_MINI_CLICK_V2
    #define DISABLE_USB_SET_UID_DEV_PASSWORD_COMMANDS
    #define KNOCK_SETTINGS_CHANGE_PREVENT_WHEN_CARD_INSERTED

    #define MINI_PREPROD_KICKSTARTER_SETUP
    #define ENABLE_CREDENTIAL_MANAGEMENT                    // WARNING: requires a new resource bundle.img with additional strings
    #define REPLACE_FAVORITES_WITH_CREDENTIAL_MANAGEMENT    // replaces favorites selection menu with creds management menu
    #define MINI_HARDENED_FW
#elif defined(MINI_KICKSTARTER_SETUP)
    #define MINI_VERSION
    #define FLASH_CHIP_8M
    #define DATA_STORAGE_EN
    #define HARDWARE_MINI_CLICK_V2
    #define DISABLE_USB_SET_UID_DEV_PASSWORD_COMMANDS
    #define KNOCK_SETTINGS_CHANGE_PREVENT_WHEN_CARD_INSERTED
#elif defined(MINI_KICKSTARTER_SETUP_HARDENED_CREDENTIAL_MANAGEMENT)
    //#define STACK_DEBUG
    #define MINI_VERSION
    #define FLASH_CHIP_8M
    //#define DATA_STORAGE_EN
    #define HARDWARE_MINI_CLICK_V2
    #define DISABLE_USB_SET_UID_DEV_PASSWORD_COMMANDS
    #define KNOCK_SETTINGS_CHANGE_PREVENT_WHEN_CARD_INSERTED

    #define MINI_PREPROD_KICKSTARTER_SETUP
    #define ENABLE_CREDENTIAL_MANAGEMENT                    // WARNING: requires a new resource bundle.img with additional strings
    #define REPLACE_FAVORITES_WITH_CREDENTIAL_MANAGEMENT    // replaces favorites selection menu with creds management menu
    #define MINI_HARDENED_FW
#endif

/* Features depending on the mooltipass version */
#ifndef MINI_VERSION
    #define UNLOCK_WITH_PIN_FUNCTIONALITY
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
// Used for oled display
//#define OLED_DEBUG_OUTPUT_USB

/**************** ENABLING TESTS ****************/
// As they may be manually enabled as well
#ifdef PRODUCTION_TEST_ENABLED
    #define TESTS_ENABLED
#endif

/**************** PRINTF ACTIVATION ****************/
#if defined(OLED_DEBUG_OUTPUT_USB) || defined(HW_TEST_PROC) || defined(PRODUCTION_TEST_ENABLED) || defined(DEBUG_SMC_SCREEN_PRINT) || defined(DEBUG_SMC_USB_PRINT) || defined(FLASH_TEST_DEBUG_OUTPUT_USB) || defined(GENERAL_LOGIC_OUTPUT_USB) || defined(CMD_PARSER_USB_DEBUG_OUTPUT) || defined(USB_DEBUG_OUTPUT)
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
// Mooltipass standard: Uncomment to go to the original boot loader when smart card inserted at boot
// Mooltipass mini: Uncomment to go to the original boot loader when wheel is pressed
//#define AVR_BOOTLOADER_PROGRAMMING

/**************** SMARTCARD FUSE VERSION ****************/
// First smart card sent to the first contributors: should not be uncommented but left for reference
//#define SMARTCARD_FUSE_V1

/**************** PIN HANDLING ******************/
// Uncomment to have the GUI pin routines return SMARTCARD_DEFAULT_PIN as a PIN
//#define NO_PIN_CODE_REQUIRED

/************** SMARTCARD FORMATING **************/
// Uncomment to prevent mooltipass card formatting (done once when the card is blank and inserted in the mooltipass)
//#define DISABLE_MOOLTIPASS_CARD_FORMATTING

/************** SET PASSWORD FUNCTIONS **************/
// Uncomment to remove USB set UID & device password functions (example: UID & KEY flashed by programmer at mass production)
//#define DISABLE_USB_SET_UID_DEV_PASSWORD_COMMANDS

/************** CREDENTIALS ON CARD FUNCTIONS **************/
// Uncomment to disable single credential storage on the card itself
//#define DISABLE_SINGLE_CREDENTIAL_ON_CARD_STORAGE

/************** KNOCK SETTING PROTECTION **************/
// Uncomment to prevent knock settings changes when card is inserted
//#define KNOCK_SETTINGS_CHANGE_PREVENT_WHEN_CARD_INSERTED

/***************** CRITICAL CALLBACKS *****************/
// Uncomment to allow sending messages through USB for critical callbacks (memoryBoundaryErrorCallback and such)
//#define USB_MESSAGES_FOR_CRITICAL_CALLBACKS

/***************** ACCELEROMETER RELATED FUNCTIONALITIES *****************/
// Uncomment to disable accelerometer related functionalities
//#define NO_ACCELEROMETER_FUNCTIONALITIES

/***************** SECURITY RELATED FUNCTIONALITIES *****************/
// Uncomment to request password at boot when wheel is pressed to enable USB & admin functionalities
//#define PASSWORD_FOR_USB_AND_ADMIN_FUNCS

/************** MILLISECOND DEBUG TIMER ***************/
//#define ENABLE_MILLISECOND_DBG_TIMER

/************** LOW LEVEL MEMORY BOUNDARY CHECKS ***************/
#ifndef MINI_BOOTLOADER
    #define MEMORY_BOUNDARY_CHECKS
#endif

/************** TESTS ENABLING ***************/
// Comment to disable test calls
//#define TESTS_ENABLED

/************** MOOLTIPASS DEMOS ***************/
// Uncomment to set screen saver as default image
//#define MINI_DEMO_VIDEO

/************** FIRMWARE HARDENING ***************/
// Various hardening-related features

// Uncomment to globally enable all cleanup & hardening features
//#define MINI_HARDENED_FW

// Uncomment to allow detection of button press at boot-time
//#define MINI_BUTTON_AT_BOOT

// Uncomment to disable screensaver
//#define DISABLE_SCREENSAVER

// Uncomment to disable password compare over USB
// #define DISABLE_USB_CMD_CHECK_PASSWORD

// Uncomment to disable RNG over USB
// #define DISABLE_USB_CMD_GET_RANDOM_NUMBER

// Uncomment to restrict memory management mode to a device booted 
// with the wheel pressed
// #define MINI_RESTRICT_MEMORYMGMT // Requires MINI_BUTTON_AT_BOOT

// set all hardening and cleanup features at once
#if defined(MINI_VERSION) && defined(MINI_HARDENED_FW)
    /* features from stock firmware */
    #define NO_ACCELEROMETER_FUNCTIONALITIES          // saves about 782 bytes
    #define DISABLE_SINGLE_CREDENTIAL_ON_CARD_STORAGE // saves about 168 bytes
    #define DISABLE_FUNCTIONAL_TEST                   // saves about 466 bytes
    #define SKIP_TUTORIAL                             // saves about 62 bytes

    /* specific cleanup & hardening features */
    #define MINI_BUTTON_AT_BOOT                 // uses about 40 bytes
    #define DISABLE_SCREENSAVER                 // saves about 368 bytes

    /* USB command support hardening */
    #define DISABLE_USB_CMD_CHECK_PASSWORD      // saves about 152 bytes
    #define DISABLE_USB_CMD_GET_RANDOM_NUMBER   // saves about 20 bytes

    /* USB command restriction to boot with wheel pressed */
    #define MINI_RESTRICT_MEMORYMGMT            // uses about 10 bytes
#endif

// enforce feature requirements
#if defined(MINI_RESTRICT_MEMORYMGMT)
    #define MINI_BUTTON_AT_BOOT
#endif

/**************** HW MACROS ****************/
#define CPU_PRESCALE(n)         (CLKPR = 0x80, CLKPR = (n))

/**************** DEFINES FIRMWARE ****************/
#define AES_KEY_LENGTH          256
#define AES_BLOCK_SIZE          128
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
enum return_type_t              {RETURN_NOK = -1, RETURN_OK = 0, RETURN_BACK = 2};
enum flash_ret_t                {RETURN_INVALID_PARAM = -2, RETURN_WRITE_ERR = -3, RETURN_READ_ERR = -4, RETURN_NO_MATCH = -5};
enum justify_t                  {OLED_LEFT  = 0, OLED_RIGHT = 1, OLED_CENTRE = 2};
enum scrolling_flag_t           {OLED_SCROLL_NONE = 0, OLED_SCROLL_UP = 1, OLED_SCROLL_DOWN = 2, OLED_SCROLL_FLIP = 3};
enum wheel_action_ret_t         {WHEEL_ACTION_NONE = 0, WHEEL_ACTION_UP = 1, WHEEL_ACTION_DOWN = 2, WHEEL_ACTION_SHORT_CLICK = 3, WHEEL_ACTION_LONG_CLICK = 4, WHEEL_ACTION_CLICK_UP = 5, WHEEL_ACTION_CLICK_DOWN = 6, WHEEL_ACTION_DISCARDED = 7};
enum mini_input_yes_no_ret_t    {MINI_INPUT_RET_TIMEOUT = -1, MINI_INPUT_RET_NONE = 0, MINI_INPUT_RET_NO = 1, MINI_INPUT_RET_YES = 2, MINI_INPUT_RET_BACK = 3};
enum led_animation_type_t       {ANIM_NONE = 0x00, ANIM_FADE_IN_FADE_OUT_1_TIME = 0x01, ANIM_PULSE_UP_RAMP_DOWN = 0x02, ANIM_TURN_AROUND = 0x04};
enum acc_algo_ret_t             {ACC_RET_NOTHING = 0, ACC_RET_MOVEMENT, ACC_RET_KNOCK};

/**************** TYPEDEFS ****************/
typedef void (*bootloader_f_ptr_type)(void);
typedef int8_t RET_TYPE;

/**************** VERSION DEFINES ***************/
#ifndef MOOLTIPASS_VERSION
    #define MOOLTIPASS_VERSION "v1.2"
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
#if defined(HARDWARE_MINI_CLICK_V1) || defined(HARDWARE_MINI_CLICK_V2)
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
    
    // Click wheel
    #ifdef HARDWARE_MINI_CLICK_V1
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
    #elif defined(HARDWARE_MINI_CLICK_V2)
        #define PORTID_WHEEL_A      PORTF1
        #define PIN_WHEEL_A         PINF
        #define PORTID_WHEEL_B      PORTF5
        #define PIN_WHEEL_B         PINF
        #define PORT_WHEEL_A        PORTF
        #define PORT_WHEEL_B        PORTF
        #define DDR_WHEEL_A         DDRF
        #define DDR_WHEEL_B         DDRF
        #define PORTID_CLICK        PORTF4
        #define PORT_CLICK          PORTF
        #define DDR_CLICK           DDRF
        #define PIN_CLICK           PINF
    #endif

    // Accelerometer
    #if defined(HARDWARE_MINI_CLICK_V2)
        #define PORTID_ACC_INT      PORTD0
        #define PORT_ACC_INT        PORTD
        #define DDR_ACC_INT         DDRD
        #define PIN_ACC_INT         PIND
        #define PORTID_ACC_SS       PORTD1
        #define PORT_ACC_SS         PORTD
        #define DDR_ACC_SS          DDRD
    #endif

    // LEDs
    #if defined(HARDWARE_MINI_CLICK_V2)
        #define PORTID_LED_MOS      PORTC6
        #define PORT_LED_MOS        PORTC
        #define DDR_LED_MOS         DDRC
        #define PORTID_LED_1        PORTE6
        #define PORT_LED_1          PORTE
        #define DDR_LED_1           DDRE
        #define PORTID_LED_2        PORTB6
        #define PORT_LED_2          PORTB
        #define DDR_LED_2           DDRB
        #define PORTID_LED_3        PORTF6
        #define PORT_LED_3          PORTF
        #define DDR_LED_3           DDRF
        #define PORTID_LED_4        PORTF7
        #define PORT_LED_4          PORTF
        #define DDR_LED_4           DDRF
    #endif
#endif

#endif /* DEFINES_H_ */
