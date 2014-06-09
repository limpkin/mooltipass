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

//    usbKeybPutStr("   ");

    switch(datacmd)
    {
        // ping command
        case CMD_PING :
            pluginSendMessage(CMD_PING, 0, (char*)incomingData);
            break;

        // version command
        case CMD_VERSION :
            incomingData[0] = 0x01;
            incomingData[1] = 0x01;
            pluginSendMessage(CMD_VERSION, 2, (char*)incomingData);
            break;

        default : break;
    }
}

