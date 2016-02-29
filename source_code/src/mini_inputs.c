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
/*! \file   mini_inputs.c
 *  \brief  Joystick & wheel functions
 *  Created: 15/2/2016
 *  Copyright [2016] [Mathieu Stephan]
 */
#include <util/atomic.h>
#include "defines.h"
// This code is only used for the mooltipass mini
#ifdef MINI_VERSION
// Joystick scan list
uint8_t joystick_scan_defines[] = {PORTID_JOY_UP, PORTID_JOY_DOWN, PORTID_JOY_LEFT, PORTID_JOY_RIGHT, PORTID_JOY_CENTER};
// Joystick counter
uint8_t joystick_counters[8];
// Joystick states
uint8_t joystick_return[8];
// Wheel click counter
uint8_t wheel_click_counter;
// Wheel click return
uint8_t wheel_click_return;


/*! \fn     initMiniInputs(void)
*   \brief  Init Mooltipass mini inputs
*/
void initMiniInputs(void)
{
    // Joystick
    for (uint8_t i = 0; i < sizeof(joystick_scan_defines); i++)
    {
        DDR_JOYSTICK &= ~(1 << joystick_scan_defines[i]);
        PORT_JOYSTICK |= (1 << joystick_scan_defines[i]);
    }        
    
    // Wheel
    DDR_CLICK &= ~(1 << PORTID_CLICK);
    PORT_CLICK |= (1 << PORTID_CLICK);
    DDR_WHEEL_A &= ~(1 << PORTID_WHEEL_A);
    PORT_WHEEL_A |= (1 << PORTID_WHEEL_A);
    DDR_WHEEL_B &= ~(1 << PORTID_WHEEL_B);
    PORT_WHEEL_B |= (1 << PORTID_WHEEL_B);
}

/*! \fn     scanMiniInputsDetect(void)
*   \brief  Joystick & wheel debounce called by 1ms interrupt
*/
void scanMiniInputsDetect(void)
{
    uint8_t* current_direction_counter_pt;
    uint8_t* current_direction_return_pt;
    uint8_t current_direction;
    
    // Wheel click
    if (!(PIN_CLICK & (1 << PORTID_CLICK)))
    {
        if (wheel_click_counter == 50)
        {
            wheel_click_return = RETURN_JDETECT;
            wheel_click_counter++;
        }
        else if (wheel_click_counter != 0xFF)
        {
            wheel_click_counter++;
        }
    }
    else
    {
        if (wheel_click_return == RETURN_DET)
        {
            wheel_click_return = RETURN_JRELEASED;
        }
        else if (wheel_click_return != RETURN_JRELEASED)
        {
            wheel_click_return = RETURN_REL;
        }
        wheel_click_counter = 0;
    }
    
    // Joystick
    for (uint8_t i = 0; i < sizeof(joystick_scan_defines); i++)
    {
        current_direction = joystick_scan_defines[i];
        current_direction_return_pt = &joystick_return[current_direction];
        current_direction_counter_pt = &joystick_counters[current_direction];
        
        // Detect if pressed
        if (!(PIN_JOYSTICK & (1 << current_direction)))
        {
            if (*current_direction_counter_pt == 50)
            {
                // We must make sure the user detected that the button was released before setting it as detected!
                if (*current_direction_return_pt != RETURN_JRELEASED)
                {
                    *current_direction_return_pt = RETURN_JDETECT;
                    (*current_direction_counter_pt)++;
                }
            }
            else if (*current_direction_counter_pt != 0xFF)
            {
                (*current_direction_counter_pt)++;
            }
        }
        else
        {
            if (*current_direction_return_pt == RETURN_DET)
            {
                *current_direction_return_pt = RETURN_JRELEASED;
            }
            else if (*current_direction_return_pt != RETURN_JRELEASED)
            {
                *current_direction_return_pt = RETURN_REL;
            }
            *current_direction_counter_pt = 0;
        }
    }
}

/*! \fn     isWheelClicked(void)
*   \brief  Know if the wheel is clicked
*   \return just released/pressed, (non)detected
*/
RET_TYPE isWheelClicked(void)
{
    // This copy is an atomic operation
    volatile RET_TYPE return_val = wheel_click_return;

    if ((return_val != RETURN_DET) && (return_val != RETURN_REL))
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (wheel_click_return == RETURN_JDETECT)
            {
                wheel_click_return = RETURN_DET;
            }
            else if (wheel_click_return == RETURN_JRELEASED)
            {
                wheel_click_return = RETURN_REL;
            }
        }
    }

    return return_val;
}

/*! \fn     isMiniDirectionPressed(uint8_t direction)
*   \brief  Know if a direction is pressed
*   \param  direction   The direction (see mini_inputs.h)
*   \return just released/pressed, (non)detected
*/
RET_TYPE isMiniDirectionPressed(uint8_t direction)
{
    // This copy is an atomic operation
    volatile RET_TYPE return_val = joystick_return[direction];

    if ((return_val != RETURN_DET) && (return_val != RETURN_REL))
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (joystick_return[direction] == RETURN_JDETECT)
            {
                joystick_return[direction] = RETURN_DET;
            }
            else if (joystick_return[direction] == RETURN_JRELEASED)
            {
                joystick_return[direction] = RETURN_REL;
            }
        }
    }

    return return_val;
}
#endif