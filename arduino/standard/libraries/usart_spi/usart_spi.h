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

/* Copyright (c) 2014 Darran Hunt. All rights reserved. */

#ifndef _SPI_H_
#define _SPI_H_

#include <stdio.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <Arduino.h>
#include <avr/interrupt.h>

/* 
 * BAUD = Fosc / 2*(URR1 + 1)
 *      = 16000000 / (2*URR1 + 2)
 *
 * URR1 = (Fosc / 2*BAUD) - 1
 */
#define SPI_BAUD_8_MHZ		0
#define SPI_BAUD_4_MHZ		1
#define SPI_BAUD_2_MHZ		3
#define SPI_BAUD_1_MHZ		7
#define SPI_BAUD_800_KHZ	9
#define SPI_BAUD_500_KHZ	15
#define SPI_BAUD_400_KHZ	19
#define SPI_BAUD_100_KHZ	79

class USARTSPI {
public:
    USARTSPI(uint16_t baud=SPI_BAUD_8_MHZ) : _baud(baud) {};
    inline static uint8_t transfer(uint8_t data);
    void begin();
    void setBaud(uint16_t baud);
private:
    uint16_t _baud;
};

uint8_t USARTSPI::transfer(uint8_t data) 
{
    /* Wait for empty transmit buffer */
    while ( !( UCSR1A & (1<<UDRE1)) );
    UDR1 = data;
    /* Wait for data to be received */
    while ( !(UCSR1A & (1<<RXC1)) );
    return UDR1;
}

#endif
