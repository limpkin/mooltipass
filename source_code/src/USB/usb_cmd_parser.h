#ifndef _USB_CMD_PARSER_H_
#define _USB_CMD_PARSER_H_

#include <stdint.h>

/* USB mooltipass hid commands */
#define CMD_DEBUG		0x01
#define CMD_PING		0x02
#define CMD_VERSION		0x03

/* Packet format defines */
#define HID_LEN_FIELD	0x00
#define HID_TYPE_FIELD	0x01
#define HID_DATA_START	0x02

void usbProcessIncoming(uint8_t* incomingData);

#endif