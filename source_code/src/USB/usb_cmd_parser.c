#include "usb_cmd_parser.h"
#include <stdint.h>
#include "usb.h"

void usbProcessIncoming(uint8_t* incomingData)
{
	// Data buffer to send packet
	uint8_t dataBuffer[RAWHID_TX_SIZE];
	
    // Get data len
    // uint8_t datalen = incomingData[HID_LEN_FIELD];

    // Get data cmd
    uint8_t datacmd = incomingData[HID_TYPE_FIELD];

    usbPrintf_P(PSTR("Data Received cmd: %i"), datacmd);

    switch(datacmd)
    {
		// ping command
        case CMD_PING :
			dataBuffer[HID_LEN_FIELD] = 0;
			dataBuffer[HID_TYPE_FIELD] = CMD_PING;
			usbRawHidSend(dataBuffer, USB_WRITE_TIMEOUT);
            break;

		// version command
        case CMD_VERSION :
			dataBuffer[HID_LEN_FIELD] = 2;
			dataBuffer[HID_TYPE_FIELD] = CMD_VERSION;
			dataBuffer[HID_DATA_START] = 0x01;
			dataBuffer[HID_DATA_START+1] = 0x01;
			usbRawHidSend(dataBuffer, USB_WRITE_TIMEOUT);
            break;

        default : break;
    }
}

