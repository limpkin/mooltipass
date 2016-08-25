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
#include <util/atomic.h>
#include <stdint.h>
#include "logic_eeprom.h"
#include "mini_leds.h"
#include "defines.h"
#include "pwm.h"

#if defined(LEDS_ENABLED_MINI)
volatile uint16_t led_animation_var1;
volatile uint8_t led_animation_var2;
volatile uint8_t led_animation_var3;
volatile uint8_t led_animation;

 
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

/*! \fn     miniLedsSetAnimation(uint8_t animation)
 *  \brief  Set new LED animation
 *  \param  animation The animation to be played
 */
void miniLedsSetAnimation(uint8_t animation)
{
    // Apply animation mask in case user doesn't want this particular one
    animation &= getMooltipassParameterInEeprom(MINI_LED_ANIM_MASK_PARAM);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // Reset global vars
        led_animation_var1 = 0;
        led_animation_var2 = 0;
        led_animation_var3 = 0;
        led_animation = animation;
        setPwmDc(led_animation_var1);

        // Initial state for animations
        if ((led_animation == ANIM_FADE_IN_FADE_OUT_1_TIME) || (led_animation == ANIM_PULSE_UP_RAMP_DOWN))
        {
            miniSetLedStates(0x0F);
        }
        else if (led_animation == ANIM_TURN_AROUND)
        {
            led_animation_var2 = 0x01;
            led_animation_var1 = 0x3000;
            miniSetLedStates(led_animation_var2);
        }
    }
}

/*! \fn     miniLedsAnimationTick(void)
 *  \brief  Function called for the animation tick
 */
void miniLedsAnimationTick(void)
{
    if (led_animation == ANIM_FADE_IN_FADE_OUT_1_TIME)
    {
        if (led_animation_var2 == FALSE)
        {
            led_animation_var1+= 32;

            if (led_animation_var1 == 0)
            {
                led_animation_var2 = TRUE;
                return;
            }
        }
        else
        {
            led_animation_var1 -= 32;

            if (led_animation_var1 == 0)
            {
                led_animation = ANIM_NONE;
            }
        }
        setPwmDc(led_animation_var1);
    }
    else if (led_animation == ANIM_PULSE_UP_RAMP_DOWN)
    {
        led_animation_var1 -= 32;
        setPwmDc(led_animation_var1);
    }
    else if (led_animation == ANIM_TURN_AROUND)
    {
        if (led_animation_var1 == 0x2000)
        {
            led_animation_var3 = 0;
            if ((led_animation_var2 & 0x08) != 0)
            {
                led_animation_var2 = 0x01;
            } 
            else
            {
                led_animation_var2 = led_animation_var2 << 1;
            }
        }

        if (led_animation_var3 == 0)
        {
            led_animation_var1 += 64;

            if (led_animation_var1 == 0)
            {
                led_animation_var3 = 1;
                return;
            }
        } 
        else
        {
            led_animation_var1 -= 64;
        }

        miniSetLedStates(led_animation_var2);
        setPwmDc(led_animation_var1);
    }
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
