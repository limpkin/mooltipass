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
/*!  \file     timer_manager.h
*    \brief    Timer logic
*    Created:  12/8/2014
*    Author:   Mathieu Stephan
*/


#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_

#include "defines.h"
#include <stdint.h>

// Prototypes
void timerManagerTick(void);
void timerBasedDelayMs(uint16_t ms);
void activateTimer(uint8_t uid, uint16_t val);
RET_TYPE hasTimerExpired(uint8_t uid, uint8_t clear);

// Structs
typedef struct
{
    uint16_t timer_val;
    uint8_t flag;
} timerEntry_t;

// Defines
#define NUMBER_OF_TIMERS    7
#define TIMER_LIGHT         0
#define TIMER_SCREEN        1
#define TIMER_USERINT       2
#define TIMER_CAPS          3
#define TIMER_CREDENTIALS   4
#define TIMER_PASS_CHECK    5
#define TIMER_WAIT_FUNCTS   6


#endif /* TIMER_MANAGER_H_ */