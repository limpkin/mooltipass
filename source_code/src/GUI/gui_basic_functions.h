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
/*!  \file     gui_basic_functions.h
*    \brief    General user interface - basic functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/


#ifndef GUI_BASIC_FUNCTIONS_H_
#define GUI_BASIC_FUNCTIONS_H_

int8_t touchWheelIntefaceLogic(RET_TYPE touch_detection_result);
int8_t getTouchedPositionAnswer(uint8_t led_mask);
void activityDetectedRoutine(void);
void guiMainLoop(void);

#endif /* GUI_BASIC_FUNCTIONS_H_ */