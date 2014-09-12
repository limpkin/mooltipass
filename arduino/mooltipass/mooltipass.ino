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


/*
 * Mooltipass Oled Demo
 */

#include <usart_spi.h>
#include <oledmp.h>
#include <util/delay.h>
#include "aqua.h"
#include "sphere.h"
#include "hackaday.h"
#include "had_mooltipass_2.h"
#include "gear.h"

USARTSPI spi(SPI_BAUD_8_MHZ);
OledMP oled(spi);	

uint32_t count=0;

void setup()
{
    _delay_ms(5000);

    Serial.begin(115200);
    Serial.println("Ready");

    spi.begin();
    Serial.println("SPI ready");

    oled.begin();
    Serial.println("OLED ready");

    oled.setColour(2);
    oled.setBackground(0);

    oled.bitmapDraw(0,0, &image_HaD_Mooltipass_2);

    delay(2000);
    oled.clear();
    oled.printf(F("Mooltipass\n\n"));
    oled.printf(F("Developed on HACK A DAY"));

    Serial.println("Finished");
}

void loop()
{
}
