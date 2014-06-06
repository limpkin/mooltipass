#ifndef _USB_H_
#define _USB_H_

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include "Descriptors.h"
#include "Config/AppConfig.h"
		
#include <LUFA/Drivers/USB/USB.h>

/* usb mooltipass hid commands */
#define CMD_DEBUG	0x01
#define CMD_PING	0x02
#define CMD_VERSION 0x03


/* setup functions */
int  usb_configured(void);
void usb_init(void);

/* send data/keys functions */
void usb_debug( char *msg );
int  usb_debug_printf( const char *fmt, ...);
void usb_send_data( uint8_t cmd, uint8_t len, uint8_t buffer[] );
void usb_keyboard_press(uint8_t key, uint8_t modifier);

/* process incoming data */
void usb_check_incoming(void);

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);
void EVENT_USB_Device_StartOfFrame(void);

#endif

