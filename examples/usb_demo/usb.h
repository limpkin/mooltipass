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

int usb_configured(void);
void usb_init(void);

void print_debug( char *msg );

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);
void EVENT_USB_Device_StartOfFrame(void);

#endif

