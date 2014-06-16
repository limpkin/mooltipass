#ifndef _USB_CMD_PARSER_H_
#define _USB_CMD_PARSER_H_

#include <stdint.h>
#include <usb.h>

/* USB mooltipass hid commands */
#define CMD_DEBUG           0x01
#define CMD_PING            0x02
#define CMD_VERSION         0x03
#define CMD_CONTEXT         0x04
#define CMD_GET_LOGIN       0x05
#define CMD_GET_PASSWORD    0x06
#define CMD_SET_LOGIN       0x07
#define CMD_SET_PASSWORD    0x08
#define CMD_CHECK_PASSWORD  0x09
#define CMD_ADD_CONTEXT     0x0A
    
/* Packet format defines     */
#define HID_LEN_FIELD       0x00
#define HID_TYPE_FIELD      0x01
#define HID_DATA_START      0x02

typedef struct {
    uint8_t len;
    uint8_t cmd;
    uint8_t body[RAWHID_RX_SIZE-2];
} usbMsg_t;

void usbProcessIncoming(uint8_t* incomingData);

#endif
