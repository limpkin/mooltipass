
#include "usb.h"

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

void EVENT_USB_Device_Connect(void)
{

}

void EVENT_USB_Device_Disconnect(void)
{
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	/* Setup HID Report Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_IN_EPADDR, EP_TYPE_INTERRUPT, BUFFER_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_OUT_EPADDR, EP_TYPE_INTERRUPT, BUFFER_EPSIZE, 1);

	/* Setup Keyboard HID Report Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(KEYBOARD_IN_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(KEYBOARD_OUT_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);   
}

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
		                    case 0:	
						// todo
                	            	break;          
                	    
		                    case 1:     
					ReportData = (uint8_t*)&KeyboardReportData;
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
				    
				    case 0 : 
						// todo
				        break;
				    case 1:
						// todo
				        break;
				}
			}

		break;
	}
}


int usb_configured(void)
{
	if (USB_DeviceState != DEVICE_STATE_Configured)
        return 1;

    return 0;    
}

void print_debug( char *msg )
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
}




