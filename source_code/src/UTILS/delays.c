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
/*!  \file     delays.c
*    \brief    Different delays used in the mooltipass
*    Created:  09/6/2014
*    Author:   Mathieu Stephan
*/ 
#include "timer_manager.h"
#include "mini_inputs.h"

/*! \fn     userViewDelay(void)
*   \brief  2secs delay for the user to view information
*/
void userViewDelay(void)
{
    timerBasedDelayMs(2000);

#ifdef MINI_VERSION
    // Discard user wheel input
    miniWheelClearDetections();
#endif
}

/*! \fn     smallForLoopBasedDelay(void)
*   \brief  Small delay used at the mooltipass start
*/
void smallForLoopBasedDelay(void)
{
    for (uint16_t i = 0; i < 20000; i++) asm volatile ("NOP");
}