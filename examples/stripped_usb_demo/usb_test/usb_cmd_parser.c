#include "usb_cmd_parser.h"

void usb_process_incoming(uint8_t* incomingData)
{
    // get data len
    uint8_t datalen = incomingData[0];

    // get data cmd
    uint8_t datacmd = incomingData[1];

    usb_debug_printf( "Data Received cmd:%i", datacmd );

    uint8_t dataBuffer[62];

    switch(datacmd)
    {
        case CMD_PING: // ping command
            dataBuffer[0] = 0x02;
            usb_send_data( CMD_PING, 1, dataBuffer);
            break;

        case CMD_VERSION: // version command
            dataBuffer[0] = 0x01;   // major version
            dataBuffer[1] = 0x01;   // minor version
            usb_send_data( CMD_VERSION, 2, dataBuffer);
            break;

        case CMD_RAW: // raw command
        	rawDataCallBack(&dataBuffer[2], datalen);
        	break;
        	
        /* add new commands here */

        break;

    }
}

