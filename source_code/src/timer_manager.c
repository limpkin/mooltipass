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
/*!  \file     timer_manager.c
*    \brief    Timer logic
*    Created:  12/8/2014
*    Author:   Mathieu Stephan
*/
#include "timer_manager.h"
#include <util/atomic.h>
#include "defines.h"

// Timers array
timerEntry_t context_timers[NUMBER_OF_TIMERS];


/*!	\fn		timerManagerTick(void)
*	\brief	Function called by interrupt every ms
*/
void timerManagerTick(void)
{
    uint8_t i;
    
    for (i = 0; i < NUMBER_OF_TIMERS; i++)
    {
        if (context_timers[i].timer_val != 0)
        {
            if (context_timers[i].timer_val-- == 1)
            {
                context_timers[i].flag = TRUE;
            }
        }
    }
}

/*!	\fn		isTimerFlagPresent(uint8_t uid)
*	\brief	Know if a timer expired and clear the flag if so
*   \param  uid Unique ID
*   \return RETURN_OK or RETURN_NOK
*/
RET_TYPE isTimerFlagPresent(uint8_t uid)
{
    // Compare is done in one cycle
    if (context_timers[uid].flag == TRUE)
    {
        context_timers[uid].flag = FALSE;
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*!	\fn		isTimerRunning(uint8_t uid)
*	\brief	Know if a timer is running
*   \param  uid Unique ID
*   \return RETURN_OK or RETURN_NOK
*/
RET_TYPE isTimerRunning(uint8_t uid)
{    
    // Compare is done in one cycle
    if (context_timers[uid].timer_val > 0)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*!	\fn		activateTimer(uint8_t uid, uint16_t val)
*	\brief	Activate timer
*   \param  uid Unique ID
*   \param  val Delay
*/
void activateTimer(uint8_t uid, uint16_t val)
{
    // Compare is done in one cycle
    if (context_timers[uid].timer_val != val)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            context_timers[uid].timer_val = val;
            context_timers[uid].flag = FALSE;
        }
    }
}