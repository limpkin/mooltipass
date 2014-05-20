/* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*! \file   mooltipass.c
 *  \brief  main file
 *  Copyright [2014] [Mathieu Stephan]
 */

#include <util/delay.h>
#include <stdlib.h>
#include <avr/io.h>
#include <stdio.h>
#include "usb.h"
#include "mooltipass.h"
#include "defines.h"


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
        case 0x02: // ping command
            dataBuffer[0] = 0x02;
            usb_send_data( CMD_PING, 1, dataBuffer);
            break;

        case 0x03: // version command
            dataBuffer[0] = 0x01;   // major version
            dataBuffer[1] = 0x01;   // minor version
            usb_send_data( CMD_VERSION, 2, dataBuffer);
            break;

        break;

    }
}


int main(void)
{
    CPU_PRESCALE(0);                    // Set for 16MHz clock
    _delay_ms(500);                     // Let the power settle
    usb_init();                         // Initialize USB controller
    while(!usb_configured()); // Wait for host to set configuration

    while (1)
    {
        usb_check_incoming(usb_process_incoming);
    }
    
}
