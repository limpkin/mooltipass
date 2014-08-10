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
#include "defines.h"
#include <string.h>
#include "touch.h"
#include "gui.h"


/*! \fn     checkTSPres()
*   \brief  Check that the AT42QT2120 is here
*   \return RETURN_OK or RETURN_NOK
*/
RET_TYPE checkTSPres(void)
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

/*! \fn     isWheelTouched()
*   \brief  Check if the touch wheel is touched
*   \return RETURN_OK or RETURN_NOK
*/
RET_TYPE isWheelTouched(void)
{
    uint8_t temp_byte;
    
    readDataFromTS(REG_AT42QT_KEY_STAT1, &temp_byte);
    readDataFromTS(REG_AT42QT_DET_STAT, &temp_byte);   
    if (temp_byte & 0x02)
    {
        return RETURN_OK;
    } 
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     isButtonTouched()
*   \brief  Check if the a touch button is touched
*   \return RETURN_OK or RETURN_NOK
*/
RET_TYPE isButtonTouched(void)
{
    uint8_t temp_byte;
    
    readDataFromTS(REG_AT42QT_KEY_STAT2, &temp_byte);
    readDataFromTS(REG_AT42QT_DET_STAT, &temp_byte);    
    if ((temp_byte & 0x01) && !(temp_byte & 0x02))
    {
        return RETURN_OK;
    } 
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     getTouchedButton()
*   \brief  Find which button is touched
*   \return LEFT_BUTTON or RIGHT_BUTTON
*/
RET_TYPE getTouchedButton(void)
{
    uint8_t temp_byte;
    
    readDataFromTS(REG_AT42QT_KEY_STAT2, &temp_byte);    
    if (temp_byte & 0x02)
    {
        return LEFT_BUTTON;
    } 
    else if (temp_byte & 0x08)
    {
        return RIGHT_BUTTON;
    }
    else
    {
        return GUARD_BUTTON;
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
            writeDataToTS(REG_AT42QT_DI, 6);                                                           // Increase detection integrator value
            writeDataToTS(REG_AT42QT_KEY0_PULSE_SCL, 0x21);                                            // Oversample to gain one bit
            writeDataToTS(REG_AT42QT_KEY1_PULSE_SCL, 0x21);                                            // Oversample to gain one bit
            writeDataToTS(REG_AT42QT_KEY2_PULSE_SCL, 0x21);                                            // Oversample to gain one bit                                           
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

/*! \fn     touchDetectionRoutine()
*   \brief  Touch detection routine
*   \return Touch detection result (see touch_detect_return_t)
*/
RET_TYPE touchDetectionRoutine(void)
{
    RET_TYPE return_val = RETURN_NO_CHANGE;
    uint8_t led_states[NB_KEYS];
    uint8_t temp_byte;
    
    // Set the LEDs on by default
    memset((void*)led_states, AT42QT2120_OUTPUT_H_VAL, NB_KEYS);
    
    if (isTouchChangeDetected())
    {
        if (isWheelTouched() == RETURN_OK)
        {
            readDataFromTS(REG_AT42QT_SLIDER_POS, &temp_byte);
                    
            if (temp_byte < 0x3F)
            {
                led_states[TOUCH_TRIGHT] = AT42QT2120_OUTPUT_L_VAL;
            }
            else if (temp_byte < 0x7F)
            {
                led_states[TOUCH_BRIGHT] = AT42QT2120_OUTPUT_L_VAL;
            }
            else if (temp_byte < 0xBF)
            {
                led_states[TOUCH_BLEFT] = AT42QT2120_OUTPUT_L_VAL;
            }
            else
            {
                led_states[TOUCH_TLEFT] = AT42QT2120_OUTPUT_L_VAL;
            }
            return_val |= RETURN_WHEEL_PRESSED;
        }
        else
        {
            return_val |= RETURN_WHEEL_RELEASED;
        }
        
        // Light the LEDs accordingly
        writeDataToTS(WHEEL_TLEFT_LED_REGISTER, led_states[TOUCH_TLEFT]);
        writeDataToTS(WHEEL_TRIGHT_LED_REGISTER, led_states[TOUCH_TRIGHT]);
        writeDataToTS(WHEEL_BLEFT_LED_REGISTER, led_states[TOUCH_BLEFT]);
        writeDataToTS(WHEEL_BRIGHT_LED_REGISTER,  led_states[TOUCH_BRIGHT]);
                
        if (isButtonTouched() == RETURN_OK)
        {
            if (getTouchedButton() == LEFT_BUTTON)
            {
                led_states[TOUCH_LEFT] = AT42QT2120_OUTPUT_L_VAL;
                return_val |= RETURN_LEFT_PRESSED;
                return_val |= RETURN_RIGHT_RELEASED;
            }
            else if(getTouchedButton() == RIGHT_BUTTON)
            {
                led_states[TOUCH_RIGHT] = AT42QT2120_OUTPUT_L_VAL;
                return_val |= RETURN_RIGHT_PRESSED;
                return_val |= RETURN_LEFT_RELEASED;
            }
            else if (getTouchedButton() == GUARD_BUTTON)
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
        
        // Light the LEDs accordingly
        writeDataToTS(LEFT_LED_REGISTER, led_states[TOUCH_LEFT]);
        writeDataToTS(RIGHT_LED_REGISTER, led_states[TOUCH_RIGHT]);
        
        // Switch on cathode if activity
        if (return_val & TOUCH_PRESS_MASK)
        {
            activityDetectedRoutine();
        }
    }
    
    return return_val;   
}