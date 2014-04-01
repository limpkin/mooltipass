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

#include "spi.h"

#define UCPHA1	1
#define XCK1	5

#define MISO2 2		// PD2
#define MOSI2 3		// PD3

void SPI::begin() 
{
    // enable pins
    UBRR1 = 0;

    DDRD |= (1 << XCK1) | (1 << MOSI2); // MOSI & SCK as ouputs (enables master mode)
    DDRD &= ~(1 << MISO2);              // MISO as input
    PORTD &= ~(1 << MISO2);             // Disable pull-up

    // Set MSPI mode of operation and SPI data mode 3.
    UCSR1C = (1 << UMSEL11) | (1 << UMSEL10) | (0 << UCPOL1) | (0 << UCPHA1);
    UCSR1B = (1<<RXEN1)|(1<<TXEN1);  // Enable receiver and transmitter
    UBRR1 = _baud;  // Set baud rate (must be done after TX is enabled)
}

void SPI::setBaud(uint16_t baud)
{
    _baud = baud;
    UBRR1 = _baud;
}

