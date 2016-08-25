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
#define LONG_PRESS_MS               1000
// After how many z samples we compute the z axis average
#define ACC_Z_AVG_NB_SAMPLES        256
// Minimum sum of the y access to reverse the display (accumulated over ACC_Z_AVG_NB_SAMPLES)
#define ACC_Y_TOTAL_NREVERSE        58*ACC_Z_AVG_NB_SAMPLES
// Same thing, but the other way around ;)
#define ACC_Y_TOTAL_REVERSE         -58*ACC_Z_AVG_NB_SAMPLES
// The maximum sum of the difference between samples & avg to run the detection algo
#define ACC_Z_MAX_AVG_SUM_DIFF      2*ACC_Z_AVG_NB_SAMPLES
// The minimum sum of the difference between samples & avg to notify there's movement
#define ACC_Z_MOVEMENT_AVG_SUM_DIFF ACC_Z_AVG_NB_SAMPLES*2
// Nb samples to timeout a second knock detection
#define ACC_Z_SECOND_KNOCK_MAX_NBS  300
// Nb samples of silence for a second knock detection
#define ACC_Z_SECOND_KNOCK_MIN_NBS  40
// Nb samples to wait before retriggering another detection
#define ACC_Z_KNOCK_REARM_WAIT      400
// Maximum width of a knock
#define ACC_Z_MAX_KNOCK_PULSE_WIDTH 20
#endif

/* MACROS */
#ifdef HARDWARE_MINI_CLICK_V2
/*! \fn     isNewAccelerometerDataReady(void)
*   \brief  Function used to check if there's new accelerometer data ready
*   \return RETURN_OK if new data ready
*/
static inline RET_TYPE isNewAccelerometerDataReady(void)
{
    if (PIN_ACC_INT & (1 << PORTID_ACC_INT))
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}
#endif

/* PROTOTYPES */
RET_TYPE miniGetWheelAction(uint8_t wait_for_action, uint8_t ignore_incdec);
RET_TYPE getNewAccelerometerDataIfAvailable(uint8_t* buffer);
RET_TYPE scanAndGetDoubleZTap(uint8_t stream_output);
RET_TYPE isMiniDirectionPressed(uint8_t direction);
void miniDirectionClearJoystickDetections(void);
RET_TYPE getMiniDirectionJustPressed(void);
RET_TYPE miniGetLastReturnedAction(void);
int8_t getWheelCurrentIncrement(void);
void miniWheelClearDetections(void);
void scanMiniInputsDetect(void);
RET_TYPE isWheelClicked(void);
RET_TYPE initMiniInputs(void);

#ifdef HARDWARE_MINI_CLICK_V2
void miniAccelerometerSendReceiveSPIData(uint8_t* data, uint8_t nbBytes);
#endif

/* GLOBAL VARS */
extern uint8_t wheel_reverse_bool;
extern uint8_t acc_detected;
#ifdef HARDWARE_MINI_CLICK_V2
    extern uint8_t knock_detection_enabled;
    extern uint8_t knock_detection_threshold;
#endif

#endif /* JOYSTICK_H_ */