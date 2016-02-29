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
/*! \file   electrical_testing.c
 *  \brief  Electrical testing functions
 *  Created: 15/2/2016
 *  Copyright [2016] [Mathieu Stephan]
 */
#include <avr/io.h>
#include "timer_manager.h"
#include "defines.h"


/*! \fn     mooltipass_standard_electrical_test(void)
 *  \brief  Mooltipass standard electrical test
 *  \param  fuse_ok Bool to know if fuses set are ok
 */
void mooltipass_standard_electrical_test(uint8_t fuse_ok)
{
    // Check if PB5 is low to start electrical test
    DDRB &= ~(1 << 5); PORTB |= (1 << 5);
    smallForLoopBasedDelay();
    if (!(PINB & (1 << 5)))
    {
        // Test result, true by default
        uint8_t test_result = TRUE;
        // Leave flash nS off
        DDR_FLASH_nS |= (1 << PORTID_FLASH_nS);
        PORT_FLASH_nS |= (1 << PORTID_FLASH_nS);
        // Set PORTD as output, leave PORTID_OLED_SS high
        DDRD |= 0xFF; PORTD |= 0xFF;
        // All other pins are input by default, run our test
        for (uint8_t i = 0; i < 4; i++)
        {
            PORTD |= 0xFF;
            smallForLoopBasedDelay();
            if (!(PINF & (0xC3)) || !(PINC & (1 << 6)) || !(PINE & (1 << 6)) || !(PINB & (1 << 4)))
            {
                test_result = FALSE;
            }
            PORTD &= (1 << PORTID_OLED_SS);
            smallForLoopBasedDelay();
            if ((PINF & (0xC3)) || (PINC & (1 << 6)) || (PINE & (1 << 6)) || (PINB & (1 << 4)))
            {
                test_result = FALSE;
            }
        }
        // PB6 as test result output
        DDRB |= (1 << 6);
        // If test successful, light green LED
        if ((test_result == TRUE) && (fuse_ok == TRUE))
        {
            PORTB |= (1 << 6);
        }
        else
        {
            PORTB &= ~(1 << 6);
        }
        while(1);
    }
}