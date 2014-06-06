#ifndef _USB_CMD_PARSER_H_
#define _USB_CMD_PARSER_H_

#include "usb.h"

/* usb mooltipass hid commands */
#define CMD_DEBUG	0x01
#define CMD_PING	0x02
#define CMD_VERSION 0x03
#define CMD_RAW		0x04

void usb_process_incoming(uint8_t* incomingData);


// callbacks
void (*rawDataCallBack)(uint8_t* data, uint8_t datalen);


#endif