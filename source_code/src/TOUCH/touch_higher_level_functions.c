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
/*! \file   touch_higher_level_functions.c
 *  \brief  Touch higher level sensing functions
 *  Copyright [2014] [Mathieu Stephan]
 */
#include "touch_higher_level_functions.h"
#include "gui_basic_functions.h"
#include "defines.h"
#include <string.h>
#include "touch.h"

// Last read wheel position
uint8_t last_raw_wheel_position;
// Last LED mask
uint8_t last_led_mask;


/*! \fn     checkTSPres()
*   \brief  Check that the AT42QT2120 is here
*   \return RETURN_OK or RETURN_NOK
*/
static inline RET_TYPE checkTSPres(void)
{
    RET_TYPE temp_return;
    uint8_t temp_byte;

    temp_return = readDataFromTS(REG_AT42QT_CHIP_ID, &temp_byte);
    if (temp_return != RETURN_OK)
    {
        return temp_return;
    }
    else if(temp_byte != AT42QT2120_ID)
    {
        return RETURN_NOK;
    }
    else
    {
        return RETURN_OK;
    }
}

/*! \fn     activateGuardKey(void)
*   \brief  Activate the guard key
*/
void activateGuardKey(void)
{
    writeDataToTS(REG_AT42QT_KEY3_PULSE_SCL, 0x00);                                             // Disable proximity sensing
    writeDataToTS(REG_AT42QT_KEY3_CTRL, AT42QT2120_GUARD_VAL|AT42QT2120_AKS_GP1_MASK);          // Set key as guard
    launchCalibrationCycle();
}

/*! \fn     activateProxDetection(void)
*   \brief  Activate the proximity detection feature
*/
void activateProxDetection(void)
{
    writeDataToTS(REG_AT42QT_KEY3_PULSE_SCL, 0x73);                                             // Activate proximity sensing
    writeDataToTS(REG_AT42QT_KEY3_CTRL, AT42QT2120_AKS_GP1_MASK);                               // Set as touch key
    launchCalibrationCycle();
}

/*! \fn     initTouchSensing()
*   \brief  Initialize AT42QT2120
*/
RET_TYPE initTouchSensing(void)
{
    #ifndef HARDWARE_V1
        RET_TYPE temp_return = checkTSPres();
        
        if (temp_return == RETURN_OK)
        {
            // Perform measurements every 16ms
            writeDataToTS(REG_AT42QT_LP, 1);
            // LED settings
            writeDataToTS(REG_AT42QT_KEY4_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (top right)
            writeDataToTS(REG_AT42QT_KEY5_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (right button)
            writeDataToTS(REG_AT42QT_KEY6_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (bottom right)
            writeDataToTS(REG_AT42QT_KEY7_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (bottom left)
            writeDataToTS(REG_AT42QT_KEY8_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (left button)
            writeDataToTS(REG_AT42QT_KEY10_CTRL, AT42QT2120_OUTPUT_H_VAL);                             // LED (top left)
            // Sensitivity settings
            #ifndef LOW_SENSITIVITY
            writeDataToTS(REG_AT42QT_DI, 6);                                                           // Increase detection integrator value
            writeDataToTS(REG_AT42QT_KEY0_PULSE_SCL, 0x21);                                            // Oversample to gain one bit
            writeDataToTS(REG_AT42QT_KEY1_PULSE_SCL, 0x21);                                            // Oversample to gain one bit
            writeDataToTS(REG_AT42QT_KEY2_PULSE_SCL, 0x21);                                            // Oversample to gain one bit
            #endif
            writeDataToTS(REG_AT42QT_TRD, 50);                                                         // Recalibration if touch detected for more than 8 seconds
            // Key settings
            writeDataToTS(REG_AT42QT_KEY0_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Enable Wheel key
            writeDataToTS(REG_AT42QT_KEY1_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Enable Wheel key
            writeDataToTS(REG_AT42QT_KEY2_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Enable Wheel key
            writeDataToTS(REG_AT42QT_KEY9_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Enable Left button
            writeDataToTS(REG_AT42QT_KEY11_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);    // Enable Right button
            writeDataToTS(REG_AT42QT_SLID_OPT, 0x40);                                                  // Enable wheel
            writeDataToTS(REG_AT42QT_SLID_OPT, 0xC0);                                                  // Enable wheel
            // Activate proximity detection
            activateProxDetection();                                                                   // Proximity detection
        }        
        return temp_return;
    #else
        return RETURN_NOK;
    #endif
}

/*! \fn     getLastRawWheelPosition(void)
*   \brief  Get the touched wheel position
*   \return The position
*/
uint8_t getLastRawWheelPosition(void)
{
    return last_raw_wheel_position;
}

/*! \fn     getWheelTouchDetectionQuarter(void)
*   \brief  Get the touch quarter
*   \return The touched quarter
*/
uint8_t getWheelTouchDetectionQuarter(void)
{
    if (last_raw_wheel_position < 0x3F)
    {
        return TOUCHPOS_WHEEL_TRIGHT;
    }
    else if (last_raw_wheel_position < 0x7F)
    {
        return TOUCHPOS_WHEEL_BRIGHT;
    }
    else if (last_raw_wheel_position < 0xBF)
    {
        return TOUCHPOS_WHEEL_BLEFT;
    }
    else
    {
        return TOUCHPOS_WHEEL_TLEFT;
    }
}

/*! \fn     touchClearCurrentDetections(void)
*   \brief  Clear interrupt line for detections
*/
void touchClearCurrentDetections(void)
{
    uint8_t temp_uint;
    readDataFromTS(REG_AT42QT_SLIDER_POS, &temp_uint);
    readDataFromTS(REG_AT42QT_DET_STAT, &temp_uint);    
    readDataFromTS(REG_AT42QT_KEY_STAT1, &temp_uint);
    readDataFromTS(REG_AT42QT_KEY_STAT2, &temp_uint);
}

/*! \fn     touchDetectionRoutine(uint8_t led_mask)
*   \brief  Touch detection routine
*   \param  led_mask    Mask containing which LEDs to switchoff
*   \return Touch detection result (see touch_detect_return_t)
*/
RET_TYPE touchDetectionRoutine(uint8_t led_mask)
{
    RET_TYPE return_val = RETURN_NO_CHANGE;
    uint8_t keys_detection_status;
    uint8_t led_states[NB_KEYS];
    uint8_t temp_bool = FALSE;
    uint8_t temp_uint;
    
    // Set the LEDs on by default
    memset((void*)led_states, AT42QT2120_OUTPUT_H_VAL, NB_KEYS);
    
    // Switch them off depending on mask    
    for (temp_uint = 0; temp_uint < NB_KEYS; temp_uint++)
    {
        if (led_mask & (1 << temp_uint))
        {
            led_states[temp_uint] = AT42QT2120_OUTPUT_L_VAL;
        }
    }
    
    if (isTouchChangeDetected())
    {
        // Set temp bool to TRUE
        temp_bool = TRUE;
        
        // Read detection status register
        readDataFromTS(REG_AT42QT_DET_STAT, &keys_detection_status);
        
        // Unused byte that needs to be read        
        readDataFromTS(REG_AT42QT_KEY_STAT1, &temp_uint);
        
        // If wheel is touched
        if (keys_detection_status & AT42QT2120_SDET_MASK)
        {
            // Get position and update global var
            readDataFromTS(REG_AT42QT_SLIDER_POS, &last_raw_wheel_position);
            
            // Update LED states
            led_states[getWheelTouchDetectionQuarter()] = AT42QT2120_OUTPUT_L_VAL;
            return_val |= RETURN_WHEEL_PRESSED;
        }
        else
        {
            return_val |= RETURN_WHEEL_RELEASED;
        }

        // Read button touched register
        readDataFromTS(REG_AT42QT_KEY_STAT2, &temp_uint);
        
        // If one button is touched
        if ((keys_detection_status & AT42QT2120_TDET_MASK) && !(keys_detection_status & AT42QT2120_SDET_MASK))
        {
            if (temp_uint & 0x02)
            {
                // Left button
                led_states[TOUCHPOS_LEFT] = AT42QT2120_OUTPUT_L_VAL;
                return_val |= RETURN_LEFT_PRESSED;
                return_val |= RETURN_RIGHT_RELEASED;
            }
            else if(temp_uint & 0x08)
            {
                // Right button
                led_states[TOUCHPOS_RIGHT] = AT42QT2120_OUTPUT_L_VAL;
                return_val |= RETURN_RIGHT_PRESSED;
                return_val |= RETURN_LEFT_RELEASED;
            }
            else
            {
                return_val |= RETURN_PROX_DETECTION;
            }
        }
        else
        {
            return_val |= RETURN_PROX_RELEASED;
            return_val |= RETURN_LEFT_RELEASED;
            return_val |= RETURN_RIGHT_RELEASED;
        }
        
        // Switch on cathode if activity
        if (return_val & TOUCH_PRESS_MASK)
        {
            activityDetectedRoutine();
        }
    }
    
    // If there's a touch change or led mask has changed
    if ((temp_bool == TRUE) || (led_mask != last_led_mask))
    {
        last_led_mask = led_mask;
        writeDataToTS(LEFT_LED_REGISTER, led_states[TOUCHPOS_LEFT]);
        writeDataToTS(RIGHT_LED_REGISTER, led_states[TOUCHPOS_RIGHT]);
        writeDataToTS(WHEEL_TLEFT_LED_REGISTER, led_states[TOUCHPOS_WHEEL_TLEFT]);
        writeDataToTS(WHEEL_TRIGHT_LED_REGISTER, led_states[TOUCHPOS_WHEEL_TRIGHT]);
        writeDataToTS(WHEEL_BLEFT_LED_REGISTER, led_states[TOUCHPOS_WHEEL_BLEFT]);
        writeDataToTS(WHEEL_BRIGHT_LED_REGISTER,  led_states[TOUCHPOS_WHEEL_BRIGHT]);
    }
    
    return return_val;   
}
