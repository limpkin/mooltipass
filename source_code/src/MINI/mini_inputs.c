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
#include "oledmini.h"
#include "defines.h"
#include "delays.h"
#include "pwm.h"
#include "spi.h"
// This code is only used for the Mooltipass mini
#ifdef MINI_VERSION
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
// index for wheel current state buffer
volatile uint8_t wheel_log_buffer_index;
// buffer containing the last wheel raw state
volatile uint8_t wheel_log_buffer[2];
// To get wheel action, discard release event
uint8_t discard_release_event = FALSE;
// Wheel direction reverse bool
uint8_t wheel_reverse_bool = FALSE;
// Last detection type returned (cleared when calling cleardetections)
RET_TYPE last_detection_type_ret = WHEEL_ACTION_NONE;
#ifdef HARDWARE_MINI_CLICK_V2
// z added value
int16_t acc_z_added;
// z average value
int8_t acc_z_average;
// z counter for average
uint16_t acc_z_avg_counter;
// sum of the differences with the average
uint16_t acc_z_cum_diff_avg;
// boolean to know if we should do the tap detection
uint8_t acc_z_tap_detect_enabled = FALSE;
// knock detection sm
uint8_t knock_detect_sm;
// knock detection internal counter
uint16_t knock_detect_counter;
// time stamp of last detect
uint16_t knock_last_det_counter;
// knock detection feature enabled bool
uint8_t knock_detection_enabled;
// knock detection threshold
uint8_t knock_detection_threshold;
// first knock width
uint8_t first_knock_width;
// accelerometer detected bool
uint8_t acc_detected = FALSE;
// accumulation for y axis
int16_t acc_y_cumulated;
#endif


/*! \fn     miniAccelerometerSendReceiveSPIData(uint8_t* data, uint8_t nbBytes)
 *  \brief  Send/Receive Data to/from the accelerometer
 *  \param  data    Pointer to the data to be sent, received data will overwrite it
 *  \param  nbBytes Number of bytes to be written
 */
#ifdef HARDWARE_MINI_CLICK_V2
void miniAccelerometerSendReceiveSPIData(uint8_t* data, uint8_t nbBytes)
{
    PORT_ACC_SS &= ~(1 << PORTID_ACC_SS);
    while(nbBytes--)
    {
        *data = spiUsartTransfer(*data);
        data++;
    }
    PORT_ACC_SS |= (1 << PORTID_ACC_SS);
}
#endif

/*! \fn     initMiniInputs(void)
*   \brief  Init Mooltipass mini inputs
*   \return Init success status
*/
RET_TYPE initMiniInputs(void)
{    
    // Wheel
    DDR_CLICK &= ~(1 << PORTID_CLICK);
    PORT_CLICK |= (1 << PORTID_CLICK);
    DDR_WHEEL_A &= ~(1 << PORTID_WHEEL_A);
    PORT_WHEEL_A |= (1 << PORTID_WHEEL_A);
    DDR_WHEEL_B &= ~(1 << PORTID_WHEEL_B);
    PORT_WHEEL_B |= (1 << PORTID_WHEEL_B);

#if defined(HARDWARE_MINI_CLICK_V2)
    // Setup PORT for the Accelerometer SS & INT
    DDR_ACC_SS |= (1 << PORTID_ACC_SS);
    PORT_ACC_SS |= (1 << PORTID_ACC_SS);
    DDR_ACC_INT &= ~(1 << PORTID_ACC_INT);

    // Fetch settings
    knock_detection_threshold = getMooltipassParameterInEeprom(MINI_KNOCK_THRES_PARAM);
    knock_detection_enabled = getMooltipassParameterInEeprom(MINI_KNOCK_DETECT_ENABLE_PARAM);

    // Query the accelerometer who am I register
    uint8_t whoAmIRequestData[] = {0x8F, 0x00};
    miniAccelerometerSendReceiveSPIData(whoAmIRequestData, sizeof(whoAmIRequestData));

    // Check Signature
    if (whoAmIRequestData[1] != 0x41)
    {
        return RETURN_NOK;
    }

    // Check for absence of interrupt as we haven't enabled the ACC yet
    if ((PIN_ACC_INT & (1 << PORTID_ACC_INT)) != 0)
    {
        return RETURN_NOK;
    }

    // Output data rate configuration to 400Hz, enable all axis
    uint8_t setDataRateCommand[] = {0x20, 0x5F};
    miniAccelerometerSendReceiveSPIData(setDataRateCommand, sizeof(setDataRateCommand));

    // Set data ready signal on INT1
    uint8_t setDataReadyOnINT1[] = {0x22, 0x01};
    miniAccelerometerSendReceiveSPIData(setDataReadyOnINT1, sizeof(setDataReadyOnINT1));

    // Send command to disable accelerometer I2C block and keep address inc
    uint8_t disableI2cBlockCommand[] = {0x23, 0x06};
    miniAccelerometerSendReceiveSPIData(disableI2cBlockCommand, sizeof(disableI2cBlockCommand));

    // Give enough time for an interrupt to come
    timerBased130MsDelay();

    // Check for interrupt presence
    if ((PIN_ACC_INT & (1 << PORTID_ACC_INT)) == 0)
    {
        return RETURN_NOK;
    }
    
    acc_detected = TRUE;
    return RETURN_OK;
#else
    // Earlier HW, always return OK
    return RETURN_OK;
#endif
}

#ifdef HARDWARE_MINI_CLICK_V2
/*! \fn     getNewAccelerometerDataIfAvailable(uint8_t* buffer)
*   \brief  Fetch new accelerometer data if there's some available
*   \param  buffer      A 6 bytes buffer to store acceleration data
*   \return RETURN_OK if the buffer was filled with new data
*/
RET_TYPE getNewAccelerometerDataIfAvailable(uint8_t* buffer)
{
    if (PIN_ACC_INT & (1 << PORTID_ACC_INT))
    {
        PORT_ACC_SS &= ~(1 << PORTID_ACC_SS);
        spiUsartTransfer(0xA8);
        for (uint8_t i = 0; i < 6; i++)
        {
            *buffer++ = spiUsartTransfer(0x00);
        }
        PORT_ACC_SS |= (1 << PORTID_ACC_SS);
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     scanAndGetDoubleZTap(uint8_t stream_output)
*   \brief  Fetch remaining accelerometer data and use it to detect double taps
*   \param  stream_output   TRUE to send USB packets with the current data
*   \return RETURN_OK if a double tap event was detected
*/
RET_TYPE scanAndGetDoubleZTap(uint8_t stream_output)
{
    uint8_t acc_data[10];
    acc_data[9] = 0;
    acc_data[8] = 0;

    // Is the feature actually enabled?
    if (acc_detected == FALSE)
    {
        return ACC_RET_NOTHING;
    }

    // Fetch data if there's data to be fetched
    if (getNewAccelerometerDataIfAvailable(acc_data) != RETURN_NOK)
    {
        // Get z data acceleration value
        int8_t z_data_val = (int8_t)acc_data[5];

        // Make sure we're not getting an overflow
        if (acc_z_cum_diff_avg < (UINT16_MAX - UINT8_MAX))
        {
            // Sum of the differences with the average
            if (z_data_val > acc_z_average)
            {
                acc_z_cum_diff_avg += (z_data_val - acc_z_average);
            }
            else
            {
                acc_z_cum_diff_avg += (acc_z_average - z_data_val);
            }
        }

        // Average calculations
        acc_z_added += z_data_val;
        acc_y_cumulated += (int8_t)acc_data[3];
        if (++acc_z_avg_counter == ACC_Z_AVG_NB_SAMPLES)
        {
            // Check if we need to reverse the screen
            if ((acc_y_cumulated > ACC_Y_TOTAL_NREVERSE) && (miniOledIsDisplayReversed() != FALSE))
            {
                miniOledUnReverseDisplay();
                wheel_reverse_bool = FALSE;
                setMooltipassParameterInEeprom(INVERTED_SCREEN_AT_BOOT_PARAM, FALSE);
            } 
            else if ((acc_y_cumulated < ACC_Y_TOTAL_REVERSE) && (miniOledIsDisplayReversed() == FALSE))
            {
                miniOledReverseDisplay();
                wheel_reverse_bool = TRUE;
                setMooltipassParameterInEeprom(INVERTED_SCREEN_AT_BOOT_PARAM, TRUE);
            }
            acc_y_cumulated = 0;

            // Compute average
            acc_z_average = acc_z_added / ACC_Z_AVG_NB_SAMPLES;

            // depending on the sum of the difference with avg, allow algo or not
            if (acc_z_cum_diff_avg > ACC_Z_MAX_AVG_SUM_DIFF)
            {
                acc_z_tap_detect_enabled = FALSE;
                if (stream_output != FALSE)
                {
                    acc_data[9] = 100;
                }
            } 
            else
            {
                acc_z_tap_detect_enabled = TRUE;
                if (stream_output != FALSE)
                {
                    acc_data[9] = 20;
                }
            }         
            
            // Reset vars
            acc_z_added = 0;
            acc_z_avg_counter = 0;
            acc_z_cum_diff_avg = 0;
        }

        // Current z axis corrected value
        int8_t z_cor_data_val;
        if (z_data_val > acc_z_average)
        {
            z_cor_data_val = z_data_val - acc_z_average;
        } 
        else
        {
            z_cor_data_val = acc_z_average - z_data_val;
        }
        
        // For debug purposes, send the raw value we use for our algo
        if (stream_output != FALSE)
        {
            acc_data[7] = (uint8_t)z_cor_data_val;
        }

        // Knock detection algo
        if (knock_detect_sm == 0)
        {
            if(z_cor_data_val > knock_detection_threshold)
            {
                knock_detect_sm++;
                first_knock_width = 0;
                knock_detect_counter = 0;
                knock_last_det_counter = 0;
            }
        }
        else if (knock_detect_sm == 1)
        {
            // Check if second knock
            if (z_cor_data_val > knock_detection_threshold)
            {
                // If silence period is respected
                if (((knock_detect_counter - knock_last_det_counter) > ACC_Z_SECOND_KNOCK_MIN_NBS) && (acc_z_tap_detect_enabled != FALSE) && (knock_detection_enabled != FALSE))
                {
                    if (stream_output != FALSE)
                    {
                        acc_data[9] = 0xFF;
                        usbSendMessage(CMD_STREAM_ACC_DATA, 10, acc_data);
                    }

                    // Return success
                    knock_last_det_counter = 0;
                    knock_detect_sm++;
                    return ACC_RET_KNOCK;
                }
                else
                {
                    knock_last_det_counter = knock_detect_counter;
                }

                // Check that the time spent above the threshold isn't too long
                if (first_knock_width++ > ACC_Z_MAX_KNOCK_PULSE_WIDTH)
                {
                    knock_detect_sm++;

                    if (stream_output != FALSE)
                    {
                        acc_data[9] = 50;
                    }
                }
            }

            // Second knock detection timeout
            if (knock_detect_counter++ > ACC_Z_SECOND_KNOCK_MAX_NBS)
            {
                knock_detect_sm = 0;                
            }
        }
        else if (knock_detect_sm == 2)
        {
            // Wait before retrigger
            if (knock_last_det_counter++ > ACC_Z_KNOCK_REARM_WAIT)
            {
                knock_detect_sm = 0;
            }
        }
    
        if (stream_output != FALSE)
        {
            usbSendMessage(CMD_STREAM_ACC_DATA, 10, acc_data);
        }
    }

    // Depending on the threshold, return movement or nothing
    if (acc_z_cum_diff_avg > ACC_Z_MOVEMENT_AVG_SUM_DIFF)
    {
        return ACC_RET_MOVEMENT;
    } 
    else
    {
        return ACC_RET_NOTHING;
    }
}
#endif

/*! \fn     scanMiniInputsDetect(void)
*   \brief  Joystick & wheel debounce called by 1ms interrupt
*/
void scanMiniInputsDetect(void)
{    
    uint8_t wheel_state, wheel_sm = 0;
    
    // Wheel encoder
    wheel_state = ((PIN_WHEEL_A & (1 << PORTID_WHEEL_A)) >> PORTID_WHEEL_A) | ((PIN_WHEEL_B & (1 << PORTID_WHEEL_B)) >> (PORTID_WHEEL_B-1));
    // Store it in our log
    wheel_log_buffer[(wheel_log_buffer_index++) & 0x01] = wheel_state;
    // Only if the last x samples are the same that we are going into this scan routine...
    for (uint8_t i = 1; i < 2; i++)
    {
        if (wheel_log_buffer[i] != wheel_log_buffer[0])
        {
            return;
        }
    }
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
}

/*! \fn     getWheelCurrentIncrement(void)
*   \brief  Fetch the current increment/decrement for the wheel
*   \return positive or negative depending on the scrolling
*/
int8_t getWheelCurrentIncrement(void)
{
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
        last_detection_type_ret = WHEEL_ACTION_NONE;
        wheel_click_duration_counter = 0;
        wheel_click_return = RETURN_REL;
        wheel_increment_armed = FALSE;
        wheel_cur_increment = 0;
    }
}

/*! \fn     miniGetLastReturnedAction(void)
*   \brief  Get the last returned action to another call
*   \return See wheel_action_ret_t
*/
RET_TYPE miniGetLastReturnedAction(void)
{
    return last_detection_type_ret;
}

/*! \fn     miniGetWheelAction(void)
*   \brief  Get current wheel action
*   \param  wait_for_action     Set to TRUE to wait for an action
*   \param  ignore_incdec       Ignore actions linked to wheel scrolling
*   \return See wheel_action_ret_t
*/
RET_TYPE miniGetWheelAction(uint8_t wait_for_action, uint8_t ignore_incdec)
{
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
                if ((wheel_click_duration_counter > LONG_PRESS_MS) && (discard_release_event == FALSE))
                {
                    return_val = WHEEL_ACTION_LONG_CLICK;
                }
                else if (wheel_click_return == RETURN_JRELEASED)
                {                    
                    if (wheel_cur_increment_copy == 0)
                    {
                        if (discard_release_event != FALSE)
                        {
                            discard_release_event = FALSE;
                            return_val = WHEEL_ACTION_DISCARDED;
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
                if ((return_val == WHEEL_ACTION_CLICK_DOWN) || (return_val == WHEEL_ACTION_CLICK_UP))
                {
                    discard_release_event = TRUE;
                }
                else if(return_val == WHEEL_ACTION_NONE)
                {
                    // User has been scrolling then keeping the wheel pressed: do not do anything
                }
                else
                {
                    wheel_click_return = RETURN_REL;
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
    
    last_detection_type_ret = return_val;
    return return_val;
}
#endif