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
#include <stdint.h>
#include "mini_leds.h"
#include "defines.h"
#if defined(HARDWARE_MINI_CLICK_V2)
 
/*! \fn     miniInitLeds(void)
 *  \brief  Init LEDs on the mini
 */
void miniInitLeds(void)
{
    // Setup PORT for the LEDs, switch them off
    DDR_LED_1 |= (1 << PORTID_LED_1);
    DDR_LED_2 |= (1 << PORTID_LED_2);
    DDR_LED_3 |= (1 << PORTID_LED_3);
    DDR_LED_4 |= (1 << PORTID_LED_4);

    // Switch off LEDs
    miniSetLedStates(0x00);
}

/*! \fn     miniLedsAnimationTick(void)
 *  \brief  Function called for the animation tick
 */
void miniLedsAnimationTick(void)
{
}

/*! \fn     miniSetLedStates(uint8_t leds)
 *  \brief  Set current LEDs
 *  \param  leds    4 bits bitmask for the led states
 *  \note   PWM must be correctly before/after calling this function
 */
void miniSetLedStates(uint8_t leds)
{
    uint8_t portid_leds[] = {1 << PORTID_LED_1, 1 << PORTID_LED_2, 1 << PORTID_LED_3, 1 << PORTID_LED_4};
    volatile uint8_t* port_leds[] = {&PORT_LED_1, &PORT_LED_2, &PORT_LED_3, &PORT_LED_4};

    for (uint8_t i = 0; i < 4; i++)
    {
        if (leds & (1 << i))
        {
            *(port_leds[i]) |= portid_leds[i];
        } 
        else
        {
            *(port_leds[i]) &= ~portid_leds[i];
        }
    }
}
#endif
