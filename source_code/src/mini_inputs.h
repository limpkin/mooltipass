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
#ifdef MINI_VERSION
// How many ms is considered as a long press
#define LONG_PRESS_MS           1000
#endif

/* PROTOTYPES */
RET_TYPE miniGetWheelAction(uint8_t wait_for_action, uint8_t ignore_incdec);
RET_TYPE isMiniDirectionPressed(uint8_t direction);
void miniDirectionClearJoystickDetections(void);
RET_TYPE getMiniDirectionJustPressed(void);
void miniDirectionClearDetections(void);
int8_t getWheelCurrentIncrement(void);
void miniWheelClearDetections(void);
void scanMiniInputsDetect(void);
RET_TYPE isWheelClicked(void);
RET_TYPE initMiniInputs(void);

/* GLOBAL VARS */
extern uint8_t wheel_reverse_bool;

#endif /* JOYSTICK_H_ */