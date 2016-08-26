#ifndef _USB_CMD_PARSER_H_
#define _USB_CMD_PARSER_H_

#include "flash_mem.h"
#include <stdint.h>
#include "usb.h"

/* Timeout for bundle upload */
#define BUNDLE_UPLOAD_TIMEOUT   60000

/* USB mooltipass hid commands */
// Developper plugin commands
#define CMD_TEST_ACC_PRESENCE   0x95
#define CMD_ERASE_EEPROM        0x96
#define CMD_ERASE_FLASH         0x97
#define CMD_ERASE_SMC           0x98
#define CMD_DRAW_BITMAP         0x99
#define CMD_SET_FONT            0x9A
#define CMD_USB_KEYBOARD_PRESS  0x9B
#define CMD_STACK_FREE          0x9C
#define CMD_CLONE_SMARTCARD     0x9D
#define CMD_MINI_FRAME_BUF_DATA 0x9E
#define CMD_STREAM_ACC_DATA     0x9F
// From here the commands are used
#define CMD_DEBUG               0xA0
#define CMD_PING                0xA1
#define CMD_VERSION             0xA2
#define CMD_CONTEXT             0xA3
#define CMD_GET_LOGIN           0xA4
#define CMD_GET_PASSWORD        0xA5
#define CMD_SET_LOGIN           0xA6
#define CMD_SET_PASSWORD        0xA7
#define CMD_CHECK_PASSWORD      0xA8
#define CMD_ADD_CONTEXT         0xA9
#define CMD_SET_BOOTLOADER_PWD  0xAA
#define CMD_JUMP_TO_BOOTLOADER  0xAB
#define CMD_GET_RANDOM_NUMBER   0xAC
#define CMD_START_MEMORYMGMT    0xAD
#define CMD_IMPORT_MEDIA_START  0xAE
#define CMD_IMPORT_MEDIA        0xAF
#define CMD_IMPORT_MEDIA_END    0xB0
#define CMD_SET_MOOLTIPASS_PARM 0xB1
#define CMD_GET_MOOLTIPASS_PARM 0xB2
#define CMD_RESET_CARD          0xB3
#define CMD_READ_CARD_LOGIN     0xB4
#define CMD_READ_CARD_PASS      0xB5
#define CMD_SET_CARD_LOGIN      0xB6
#define CMD_SET_CARD_PASS       0xB7
#define CMD_ADD_UNKNOWN_CARD    0xB8
#define CMD_MOOLTIPASS_STATUS   0xB9
#define CMD_FUNCTIONAL_TEST_RES 0xBA
#define CMD_SET_DATE            0xBB
#define CMD_SET_UID             0xBC
#define CMD_GET_UID             0xBD
#define CMD_SET_DATA_SERVICE    0xBE
#define CMD_ADD_DATA_SERVICE    0xBF
#define CMD_WRITE_32B_IN_DN     0xC0
#define CMD_READ_32B_IN_DN      0xC1
#define CMD_GET_CUR_CARD_CPZ    0xC2
#define CMD_CANCEL_REQUEST      0xC3
#define CMD_PLEASE_RETRY        0xC4
// Leave some space here for future commands
/******* COMMANDS FOR DATA MANAGEMENT MODE *******/
#define CMD_READ_FLASH_NODE     0xC5
#define CMD_WRITE_FLASH_NODE    0xC6
#define CMD_GET_FAVORITE        0xC7
#define CMD_SET_FAVORITE        0xC8
#define CMD_GET_STARTING_PARENT 0xC9
#define CMD_SET_STARTING_PARENT 0xCA
#define CMD_GET_CTRVALUE        0xCB
#define CMD_SET_CTRVALUE        0xCC
#define CMD_ADD_CARD_CPZ_CTR    0xCD
#define CMD_GET_CARD_CPZ_CTR    0xCE
#define CMD_CARD_CPZ_CTR_PACKET 0xCF
#define CMD_GET_FREE_SLOTS_ADDR 0xD0
#define CMD_GET_DN_START_PARENT 0xD1
#define CMD_SET_DN_START_PARENT 0xD2
#define CMD_END_MEMORYMGMT      0xD3
#define CMD_SET_USER_CHANGE_NB  0xD4
#define FIRST_CMD_FOR_DATAMGMT  CMD_READ_FLASH_NODE
#define LAST_CMD_FOR_DATAMGMT   CMD_SET_USER_CHANGE_NB
/******* COMMANDS ADDED AFTER v1 firmware *******/
#define CMD_GET_DESCRIPTION     0xD5
#define CMD_GET_USER_CHANGE_NB  0xD6
#define CMD_GET_FREE_NB_USR_SLT 0xD7
#define CMD_UNLOCK_WITH_PIN     0xD8


/* Packet format defines     */
#define HID_LEN_FIELD       0x00
#define HID_TYPE_FIELD      0x01
#define HID_DATA_START      0x02

/* Packet answers */
#define PLUGIN_BYTE_ERROR   0x00
#define PLUGIN_BYTE_OK      0x01
#define PLUGIN_BYTE_NA      0x02
#define PLUGIN_BYTE_NOCARD  0x03

/* Packet defines */
#define PACKET_EXPORT_SIZE  (RAWHID_TX_SIZE-HID_DATA_START)
#define DATA_NODE_BLOCK_SIZ 32

/* function caller IDs */
#define USB_CALLER_MAIN     0x00
#define USB_CALLER_PIN      0x01

/*** MACROS ***/
#ifdef CMD_PARSER_USB_DEBUG_OUTPUT
    #define USBPARSERDEBUGPRINTF_P(args...) usbPrintf_P(args)
#else
    #define USBPARSERDEBUGPRINTF_P(args...)
#endif

/*** STRUCTS ***/
typedef struct
{
    uint8_t len;
    uint8_t cmd;
    union
    {
        uint8_t data[RAWHID_RX_SIZE-HID_DATA_START];
        uint32_t addr;
    } body;
} usbMsg_t;

/*** PROTOTYPES ***/
RET_TYPE checkTextField(uint8_t* data, uint8_t len, uint8_t max_len);
void usbProcessIncoming(uint8_t caller_id);
RET_TYPE usbCancelRequestReceived(void);
void leaveMemoryManagementMode(void);

#endif
