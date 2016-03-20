/*
 * bootloader.c
 *
 * Created: 19/03/2016 10:00:01
 * Author : limpkin
 */ 
#include <avr/interrupt.h>
#include <avr/io.h>
#include "defines.h"
#include "usb.h"


int main(void)
{
	sei();
    initUsb();
	while(!isUsbConfigured());
    
    /* Replace with your application code */
    while (1) 
    {
        uint8_t incomingData[RAWHID_TX_SIZE];
        uint8_t i;
        
        // Try to read data from USB, return if we didn't receive anything
        if(usbRawHidRecv(incomingData) != RETURN_COM_TRANSF_OK)
        {
            i++;
        }
    }
}

