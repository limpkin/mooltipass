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
volatile timerEntry_t context_timers[TOTAL_NUMBER_OF_TIMERS];
volatile uint16_t timer_divider;


/*!	\fn		timerManagerTick(void)
*	\brief	Function called by interrupt every ms
*/
void timerManagerTick(void)
{
    uint8_t i;
    
    // Increment divider
    timer_divider++;
    
    // Loop through the timers
    for (i = 0; i < TOTAL_NUMBER_OF_TIMERS; i++)
    {
        // If it's a fast timer or if it not & timer_divider rolled over
        if ((i < NUMBER_OF_FAST_TIMERS) || (timer_divider == 0))
        {
            if (context_timers[i].timer_val != 0)
            {
                if (context_timers[i].timer_val-- == 1)
                {
                    context_timers[i].flag = TIMER_EXPIRED;
                }
            }
        }
    }
}

/*!	\fn		hasTimerExpired(uint8_t uid)
*	\brief	Know if a timer expired and clear the flag if so
*   \param  uid     Unique ID
*   \param  clear   Boolean to say if we clear the flag
*   \return RETURN_OK or RETURN_NOK
*/
RET_TYPE hasTimerExpired(uint8_t uid, uint8_t clear)
{
    // Compare is done in one cycle
    if (context_timers[uid].flag == TIMER_EXPIRED)
    {
        if (clear == TRUE)
        {
            context_timers[uid].flag = TIMER_RUNNING;
        }
        return TIMER_EXPIRED;
    }
    else
    {
        return TIMER_RUNNING;
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
            if (val == 0)
            {
                context_timers[uid].flag = TIMER_EXPIRED;
            } 
            else
            {
                context_timers[uid].flag = TIMER_RUNNING;
            }
        }
    }
}


/*!	\fn		getTimerVal(uint8_t uid)
*	\brief	Get current timer val
*   \param  uid     Unique ID
*   \return the timer val
*/
uint16_t getTimerVal(uint8_t uid)
{
    return context_timers[uid].timer_val;
}

/*!	\fn		timerBasedDelayMs(uint16_t ms)
*	\brief	Timer based ms delay
*   \param  ms  Number of ms
*/
void timerBasedDelayMs(uint16_t ms)
{
    activateTimer(TIMER_WAIT_FUNCTS, ms+1);
    while(hasTimerExpired(TIMER_WAIT_FUNCTS, TRUE) != TIMER_EXPIRED);
}

/*!	\fn		timerBased130MsDelay(void)
*	\brief	Many times in our code such delay is needed.
*/
void timerBased130MsDelay(void)
{
    timerBasedDelayMs(130);
}    

/*!	\fn		timerBased500MsDelay(void)
*	\brief	Many times in our code such delay is needed.
*/
void timerBased500MsDelay(void)
{
    timerBasedDelayMs(500);
} 