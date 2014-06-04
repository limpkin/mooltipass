#include "usb_cmd_parser.h"
#include <stdint.h>
#include "usb.h"

/*! \fn     usbProcessIncoming(uint8_t* incomingData)
*   \brief  Process the incoming USB packet
*   \param  incomingData    Pointer to the packet (can be overwritten!)
*/
void usbProcessIncoming(uint8_t* incomingData)
{   
    // Get data len
    // uint8_t datalen = incomingData[HID_LEN_FIELD];

    // Get data cmd
    uint8_t datacmd = incomingData[HID_TYPE_FIELD];

//    usbPrintf_P(PSTR("Data Received cmd: %i"), datacmd);
    
//     if (incomingData[0] == 'a')
//     {
//         usbPutstr("lapin");
//         //usbKeybPutStr("lapin");
//     }

    usbKeybPutStr("   ");

    switch(datacmd)
    {
        // ping command
        case CMD_PING :
            incomingData[HID_LEN_FIELD] = 0;
            incomingData[HID_TYPE_FIELD] = CMD_PING;
            usbRawHidSend(incomingData, USB_WRITE_TIMEOUT);
            break;

        // version command
        case CMD_VERSION :
            incomingData[HID_LEN_FIELD] = 2;
            incomingData[HID_TYPE_FIELD] = CMD_VERSION;
            incomingData[HID_DATA_START] = 0x01;
            incomingData[HID_DATA_START+1] = 0x01;
            usbRawHidSend(incomingData, USB_WRITE_TIMEOUT);
            break;

        default : break;
    }
}

