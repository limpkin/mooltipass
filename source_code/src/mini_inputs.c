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
#include <string.h>
#include "gui_basic_functions.h"
#include "logic_eeprom.h"
#include "mini_inputs.h"
#include "defines.h"
// This code is only used for the Mooltipass mini
#ifdef MINI_VERSION
#ifdef MINI_JOYSTICK
// Joystick scan list
uint8_t joystick_scan_defines[] = {PORTID_JOY_UP, PORTID_JOY_DOWN, PORTID_JOY_LEFT, PORTID_JOY_RIGHT, PORTID_JOY_CENTER};
// Joystick counter
volatile uint8_t joystick_counters[8];
// Joystick states
volatile uint8_t joystick_return[8];
#endif
// Wheel pressed duration counter
volatile uint16_t wheel_click_duration_counter;
// Wheel click counter
volatile uint8_t wheel_click_counter;
// Wheel click return
volatile uint8_t wheel_click_return;
// State machine state
uint8_t wheel_sm_states[] = {0b011, 0b001, 0b000, 0b010};
// Boot to know if we allow next increment
volatile uint8_t wheel_increment_armed = FALSE;
// Wheel current increment for caller
volatile int8_t wheel_cur_increment;
// Last wheel state machine index
volatile uint8_t last_wheel_sm;
// To get wheel action, discard release event
uint8_t discard_release_event = FALSE;
// Wheel direction reverse bool
uint8_t wheel_reverse_bool = FALSE;


/*! \fn     initMiniInputs(void)
*   \brief  Init Mooltipass mini inputs
*/
void initMiniInputs(void)
{    
    // Wheel
    DDR_CLICK &= ~(1 << PORTID_CLICK);
    PORT_CLICK |= (1 << PORTID_CLICK);
    DDR_WHEEL_A &= ~(1 << PORTID_WHEEL_A);
    PORT_WHEEL_A |= (1 << PORTID_WHEEL_A);
    DDR_WHEEL_B &= ~(1 << PORTID_WHEEL_B);
    PORT_WHEEL_B |= (1 << PORTID_WHEEL_B);
    wheel_reverse_bool = getMooltipassParameterInEeprom(WHEEL_DIRECTION_REVERSE_PARAM);
    
    // Joystick
    #ifdef MINI_JOYSTICK
    for (uint8_t i = 0; i < sizeof(joystick_scan_defines); i++)
    {
        DDR_JOYSTICK &= ~(1 << joystick_scan_defines[i]);
        PORT_JOYSTICK |= (1 << joystick_scan_defines[i]);
    }        
    #endif
}

/*! \fn     scanMiniInputsDetect(void)
*   \brief  Joystick & wheel debounce called by 1ms interrupt
*/
void scanMiniInputsDetect(void)
{    
    #ifdef MINI_JOYSTICK
    volatile uint8_t* current_direction_counter_pt;
    volatile uint8_t* current_direction_return_pt;
    uint8_t current_direction;
    #endif
    uint8_t wheel_state, wheel_sm = 0;
    
    // Wheel encoder
    wheel_state = ((PIN_WHEEL_A & (1 << PORTID_WHEEL_A)) >> PORTID_WHEEL_A) | ((PIN_WHEEL_B & (1 << PORTID_WHEEL_B)) >> (PORTID_WHEEL_B-1));
    // Find the state matching the wheel state
    for (uint8_t i = 0; i < sizeof(wheel_sm_states); i++)
    {
        if (wheel_state == wheel_sm_states[i])
        {
            wheel_sm = i;
        }
    }
    if (wheel_sm == ((last_wheel_sm+1)&0x03))
    {
        if (wheel_state == 0x00)
        {
            wheel_increment_armed = TRUE;
        }
        else if ((wheel_state == 0x03) && (wheel_increment_armed == TRUE))
        {
            if (wheel_reverse_bool != FALSE)
            {
                wheel_cur_increment++;
            } 
            else
            {
                wheel_cur_increment--;
            }
        }
        last_wheel_sm = wheel_sm;
    }
    else if (wheel_sm == ((last_wheel_sm-1)&0x03))
    {
        if (wheel_state == 0x00)
        {
            wheel_increment_armed = TRUE;
        }
        else if ((wheel_state == 0x03) && (wheel_increment_armed == TRUE))
        {
            if (wheel_reverse_bool != FALSE)
            {
                wheel_cur_increment--;
            }
            else
            {
                wheel_cur_increment++;
            }
        }
        last_wheel_sm = wheel_sm;
    }
    
    // Wheel click
    if (!(PIN_CLICK & (1 << PORTID_CLICK)))
    {
        if ((wheel_click_counter == 50) && (wheel_click_return != RETURN_JRELEASED))
        {
            wheel_click_return = RETURN_JDETECT;
        }
        if (wheel_click_counter != 0xFF)
        {
            wheel_click_counter++;
        }
        if ((wheel_click_return == RETURN_DET) || (wheel_click_return == RETURN_JDETECT))
        {
            wheel_click_duration_counter++;
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
            wheel_click_duration_counter = 0;
            wheel_click_return = RETURN_REL;
        }
        wheel_click_counter = 0;
    }
    
    // Joystick
    #ifdef MINI_JOYSTICK
    for (uint8_t i = 0; i < sizeof(joystick_scan_defines); i++)
    {
        current_direction = joystick_scan_defines[i];
        current_direction_return_pt = &joystick_return[current_direction];
        current_direction_counter_pt = &joystick_counters[current_direction];
        
        // Detect if pressed
        if (!(PIN_JOYSTICK & (1 << current_direction)))
        {
            if ((*current_direction_counter_pt == 50) && (*current_direction_return_pt != RETURN_JRELEASED))
            {
                // We must make sure the user detected that the button was released before setting it as detected!
                *current_direction_return_pt = RETURN_JDETECT;
            }
            if (*current_direction_counter_pt != 0xFF)
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
    #endif
}

/*! \fn     getWheelCurrentIncrement(void)
*   \brief  Fetch the current increment/decrement for the wheel
*   \return positive or negative depending on the scrolling
*/
int8_t getWheelCurrentIncrement(void)
{
    #ifdef MINI_WHEEL_NOT_ACTIVE
        return 0;
    #else
        int8_t return_val = 0;
        
        if (wheel_cur_increment != 0)
        {
            activityDetectedRoutine();
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                return_val = wheel_cur_increment;
                wheel_cur_increment = 0;
            }
            
        }
        
        return return_val;
    #endif
}

/*! \fn     isWheelClicked(void)
*   \brief  Know if the wheel is clicked
*   \return just released/pressed, (non)detected
*/
RET_TYPE isWheelClicked(void)
{
    #ifdef MINI_WHEEL_NOT_ACTIVE
        return RETURN_REL;
    #else
        // This copy is an atomic operation
        volatile RET_TYPE return_val = wheel_click_return;

        if ((return_val != RETURN_DET) && (return_val != RETURN_REL))
        {
            activityDetectedRoutine();
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
    #endif
}

/*! \fn     miniWheelClearDetections(void)
*   \brief  Clear current detections
*/
void miniWheelClearDetections(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        wheel_click_duration_counter = 0;
        wheel_click_return = RETURN_REL;
        wheel_cur_increment = 0;
    }
}

/*! \fn     miniGetWheelAction(void)
*   \brief  Get current wheel action
*   \param  wait_for_action     Set to TRUE to wait for an action
*   \param  ignore_incdec       Ignore actions linked to wheel scrolling
*   \return See wheel_action_ret_t
*/
RET_TYPE miniGetWheelAction(uint8_t wait_for_action, uint8_t ignore_incdec)
{
    #ifdef MINI_WHEEL_NOT_ACTIVE
        (void)wait_for_action;
        (void)ignore_incdec;
        return WHEEL_ACTION_NONE;
    #else
        RET_TYPE return_val = WHEEL_ACTION_NONE;
        int8_t wheel_cur_increment_copy = 0;

        do
        {
            // If we want to take into account wheel scrolling
            if (ignore_incdec == FALSE)
            {
                wheel_cur_increment_copy = wheel_cur_increment;
            }

            if (wheel_click_return == RETURN_JDETECT)
            {
                // When checking for actions we clear the just detected state
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
                {
                    wheel_click_return = RETURN_DET;
                }
            }
            if ((wheel_click_return == RETURN_JRELEASED) || (wheel_cur_increment_copy != 0) || (wheel_click_duration_counter > LONG_PRESS_MS))
            {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
                {
                    if (wheel_click_duration_counter > LONG_PRESS_MS)
                    {
                        return_val =  WHEEL_ACTION_LONG_CLICK;
                    }
                    else if (wheel_click_return == RETURN_JRELEASED)
                    {                    
                        if (wheel_cur_increment_copy == 0)
                        {
                            if (discard_release_event != FALSE)
                            {
                                discard_release_event = FALSE;
                            } 
                            else
                            {
                                return_val = WHEEL_ACTION_SHORT_CLICK;
                            }
                        }
                    }
                    else if (wheel_click_return == RETURN_DET)
                    {
                        if (wheel_cur_increment_copy > 0)
                        {
                            return_val = WHEEL_ACTION_CLICK_DOWN;
                        }
                        else if (wheel_cur_increment_copy < 0)
                        {
                            return_val = WHEEL_ACTION_CLICK_UP;
                        }
                    }
                    else
                    {
                        if (wheel_cur_increment_copy > 0)
                        {
                            return_val = WHEEL_ACTION_DOWN;
                        }
                        else if (wheel_cur_increment_copy < 0)
                        {
                            return_val = WHEEL_ACTION_UP;
                        }
                    }

                    // Clear detections
                    wheel_click_duration_counter = 0;
                    if ((return_val != WHEEL_ACTION_CLICK_DOWN) && (return_val != WHEEL_ACTION_CLICK_UP))
                    {
                        wheel_click_return = RETURN_REL;
                    }
                    else
                    {
                        discard_release_event = TRUE;
                    }
                    if (ignore_incdec == FALSE)
                    {
                        wheel_cur_increment = 0;
                    }                
                }
            }
        }
        while ((wait_for_action != FALSE) && (return_val == WHEEL_ACTION_NONE));

        // Don't forget to call the activity detected routine if something happened
        if (return_val != WHEEL_ACTION_NONE)
        {
            activityDetectedRoutine();
        }
    
        return return_val;
    #endif
}

#ifdef MINI_JOYSTICK
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
        activityDetectedRoutine();
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

/*! \fn     miniDirectionClearDetections(void)
*   \brief  Clear current detections
*/
void miniDirectionClearDetections(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        memset((void*)joystick_return, RETURN_REL, sizeof(joystick_return));
        wheel_click_return = RETURN_REL;
        wheel_cur_increment = 0;
    }
}

/*! \fn     miniDirectionClearJoystickDetections(void)
*   \brief  Clear current joystick detections
*/
void miniDirectionClearJoystickDetections(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        memset((void*)joystick_return, RETURN_REL, sizeof(joystick_return));
    }
}

/*! \fn     getMiniDirectionJustPressed(void)
*   \brief  Know if a direction is pressed
*   \return 0 if no button is pressed, its ID otherwise
*/
RET_TYPE getMiniDirectionJustPressed(void)
{
    volatile RET_TYPE return_val = 0;
    
    for (uint8_t i = 0; i < sizeof(joystick_scan_defines); i++)
    {
        // This copy is an atomic operation
        return_val = joystick_return[joystick_scan_defines[i]];
        
        // See if it was just pressed or released
        if ((return_val != RETURN_DET) && (return_val != RETURN_REL))
        {
            activityDetectedRoutine();
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                if (return_val == RETURN_JDETECT)
                {
                    joystick_return[joystick_scan_defines[i]] = RETURN_DET;
                }
                else if (return_val == RETURN_JRELEASED)
                {
                    joystick_return[joystick_scan_defines[i]] = RETURN_REL;
                }
            }
            if (return_val == RETURN_JDETECT)
            {
                return_val = joystick_scan_defines[i];
                break;
            }
            else
            {
                return_val = 0;
            }
        }
    }
    
    return return_val;
}
#endif
#endif