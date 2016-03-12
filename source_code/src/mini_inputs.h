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
/*! \file   mini_inputs.h
 *  \brief  Joystick & wheel functions
 *  Created: 15/2/2016
 *  Copyright [2016] [Mathieu Stephan]
 */
#include "defines.h"

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

/* DEFINES */
// Touch input returns
#define JOYSTICK_POS_RIGHT      PORTID_JOY_RIGHT
#define JOYSTICK_POS_LEFT       PORTID_JOY_LEFT
#define JOYSTICK_POS_UP         PORTID_JOY_UP
#define JOYSTICK_POS_DOWN       PORTID_JOY_DOWN
#define JOYSTICK_POS_CENTER     PORTID_JOY_CENTER
#define WHEEL_POS_CLICK         2
// Input masks
#define JOYSTICK_LEFT_MASK      (1 << JOYSTICK_POS_LEFT)
#define JOYSTICK_RIGHT_MASK     (1 << JOYSTICK_POS_RIGHT)
#define JOYSTICK_UP_MASK        (1 << JOYSTICK_POS_UP)
#define JOYSTICK_DOWN_MASK      (1 << JOYSTICK_POS_DOWN)
#define JOYSTICK_CENTER_MASK    (1 << JOYSTICK_POS_CENTER)
#define WHEEL_MASK              (1 << WHEEL_POS_CLICK)
#define LEFT_RIGHT_MASK         (JOYSTICK_LEFT_MASK | JOYSTICK_RIGHT_MASK)
#define LEFT_RIGHT_WHEEL_MASK   (JOYSTICK_LEFT_MASK | JOYSTICK_RIGHT_MASK | WHEEL_MASK)
#define JOYSTICK_WC_MASK        (JOYSTICK_UP_MASK | JOYSTICK_DOWN_MASK | JOYSTICK_LEFT_MASK | JOYSTICK_RIGHT_MASK)
#define JOYSTICK_MASK           (JOYSTICK_UP_MASK | JOYSTICK_DOWN_MASK | JOYSTICK_LEFT_MASK | JOYSTICK_RIGHT_MASK | JOYSTICK_CENTER_MASK)
#define NO_MASK                 0xFF

/* PROTOTYPES */
RET_TYPE isMiniDirectionPressed(uint8_t direction);
void miniDirectionClearJoystickDetections(void);
RET_TYPE getMiniDirectionJustPressed(void);
void miniDirectionClearDetections(void);
int8_t getWheelCurrentIncrement(void);
void scanMiniInputsDetect(void);
RET_TYPE isWheelClicked(void);
void initMiniInputs(void);

#endif /* JOYSTICK_H_ */