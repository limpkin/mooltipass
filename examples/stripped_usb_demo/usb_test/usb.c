#include "usb.h"
#include "usb_cmd_parser.h"

static USB_KeyboardReport_Data_t KeyboardReportData;

void usb_init(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	USB_Init();
	sei();
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs and stops the USB management task.
 */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host sets the current configuration
 *  of the USB device after enumeration, and configures the generic HID device endpoints.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	/* Setup HID Report Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_IN_EPADDR, EP_TYPE_INTERRUPT, GENERIC_HID_ENDPOINT_SIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_OUT_EPADDR, EP_TYPE_INTERRUPT, GENERIC_HID_ENDPOINT_SIZE, 1);

	/* Setup Keyboard HID Report Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(KEYBOARD_IN_EPADDR, EP_TYPE_INTERRUPT, KEYBOARD_ENDPOINT_SIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(KEYBOARD_OUT_EPADDR, EP_TYPE_INTERRUPT, KEYBOARD_ENDPOINT_SIZE, 1);   
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
	uint8_t* ReportData;
	uint8_t  ReportSize;
    
	/* Handle HID Class specific requests */
	switch (USB_ControlRequest.bRequest)
	{
		case HID_REQ_GetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
                Endpoint_ClearSETUP();                
                
                switch( USB_ControlRequest.wIndex )
                {
                    case INTERFACE_ID_GENERIC:	// todo
                                                break;   
                    case INTERFACE_ID_KEYBOARD: ReportData = (uint8_t*)&KeyboardReportData;
                                                ReportSize = sizeof(KeyboardReportData);
                                                Endpoint_Write_Control_Stream_LE(ReportData, ReportSize);
                                                Endpoint_ClearOUT();
                                                break;
                }
			}
			break;

		case HID_REQ_SetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSETUP();

				switch( USB_ControlRequest.wIndex )
				{
				    case INTERFACE_ID_GENERIC : // todo
				                                break;
				    case INTERFACE_ID_KEYBOARD: // todo
				                                break;
				}
			}
		    break;
	}
}

int usb_configured(void)
{
	if (USB_DeviceState != DEVICE_STATE_Configured)
    {
        return 1;
    }
    else
    {
        return 0;
    }    
}

void usb_debug( char *msg )
{
	/* Device must be connected and configured for the task to run */
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;
	
	Endpoint_SelectEndpoint(GENERIC_IN_EPADDR);

	/* Check to see if the host is ready to accept another packet */
	if (Endpoint_IsINReady())
	{
		/* Create a temporary buffer to hold the report to send to the host */
		uint8_t GenericData[BUFFER_EPSIZE];
		
		int len = strlen( msg );

		GenericData[0] = len;
		GenericData[1] = 0x01;	// debug msg
		
		for ( int c = 0; c < len; c++ )
			GenericData[c + 2] = *msg++;

		/* Write Generic Report Data */
		Endpoint_Write_Stream_LE(&GenericData, sizeof(GenericData), NULL);

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearIN();
	}	

	_delay_ms(200);
}

int usb_debug_printf( const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[60];

    int ret = vsnprintf(buf, sizeof(buf), fmt, ap);
	usb_debug(buf);

    return ret;	
}


void usb_send_data( uint8_t cmd, uint8_t len, uint8_t buffer[] )
{
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;

	Endpoint_SelectEndpoint(GENERIC_IN_EPADDR);

	if (Endpoint_IsINReady())
	{
		uint8_t GenericData[BUFFER_EPSIZE];
		
		GenericData[0] = len;
		GenericData[1] = cmd;
		
		for ( int c = 0; c < len; c++ )
			GenericData[c + 2] = buffer[c];

		Endpoint_Write_Stream_LE(&GenericData, sizeof(GenericData), NULL);

		Endpoint_ClearIN();
	}

	_delay_ms(200);
}

/*
	list of key and modifiers available: http://www.fourwalledcubicle.com/files/LUFA/Doc/140302/html/group___group___u_s_b_class_h_i_d_common.html

	keyboard presses are broken
*/

void usb_keyboard_press(uint8_t key, uint8_t modifier)
{
	USB_KeyboardReport_Data_t        KeyboardReportData;

	// setup keyboard report data
	memset(&KeyboardReportData, 0, sizeof(USB_KeyboardReport_Data_t));

	// modifier
	KeyboardReportData.Modifier = modifier;
	KeyboardReportData.KeyCode[0] = key;

	Endpoint_SelectEndpoint(KEYBOARD_IN_EPADDR);

	if ( Endpoint_IsReadWriteAllowed() )
	{
		Endpoint_Write_Stream_LE(&KeyboardReportData, sizeof(KeyboardReportData), NULL);
		Endpoint_ClearIN();		
	}

	_delay_ms(200);

}


void usb_check_incoming()
{
	uint8_t dataReceived = -1;

	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	uint8_t GenericData[BUFFER_EPSIZE];

	Endpoint_SelectEndpoint(GENERIC_OUT_EPADDR);

	// packet received?
	if ( Endpoint_IsOUTReceived() )
	{

		if ( Endpoint_IsReadWriteAllowed())
		{
			Endpoint_Read_Stream_LE( &GenericData, sizeof(GenericData), NULL);

			dataReceived = 1;
		}

		Endpoint_ClearOUT();
	}

	if ( dataReceived == 1 )
	{
		usb_process_incoming(GenericData);		
		//processDataCallBack(GenericData);
	}
}