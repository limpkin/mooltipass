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
#include "touch.h"


/*! \fn     checkTSPres()
*   \brief  Check that the AT42QT2120 is here
*   \return RETURN_OK or RETURN_NOK
*/
RET_TYPE checkTSPres(void)
{
    RET_TYPE temp_return;
    uint8_t temp_byte;

    temp_return = readDataFromTS(AT42QT2120_ADDR, REG_AT42QT_CHIP_ID, &temp_byte);
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
    
    readDataFromTS(AT42QT2120_ADDR, REG_AT42QT_KEY_STAT1, &temp_byte);
    readDataFromTS(AT42QT2120_ADDR, REG_AT42QT_DET_STAT, &temp_byte);   
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
    
    readDataFromTS(AT42QT2120_ADDR, REG_AT42QT_KEY_STAT2, &temp_byte);
    readDataFromTS(AT42QT2120_ADDR, REG_AT42QT_DET_STAT, &temp_byte);    
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
    
    readDataFromTS(AT42QT2120_ADDR, REG_AT42QT_KEY_STAT2, &temp_byte);    
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
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_LP, 1);
            // Settings for each key
            activateProxDetection();                                                                                    // Proximity detection
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY0_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Wheel
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY1_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Wheel
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY2_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Wheel           
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY4_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (top right)
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY5_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (right button)
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY6_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (bottom right)
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY7_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (bottom left)
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY8_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (left button)
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY9_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Left button
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY10_CTRL, AT42QT2120_OUTPUT_H_VAL);                             // LED (top left)
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY11_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);    // Right button
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_SLID_OPT, 0x40);                                                  // Enable wheel
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_SLID_OPT, 0xC0);                                                  // Enable wheel
            launchCalibrationCycle();                                                                                   // Launch calibration cycle
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
    uint8_t temp_byte;
    
    if (isTouchChangeDetected())
    {
        if (isWheelTouched() == RETURN_OK)
        {
            readDataFromTS(AT42QT2120_ADDR, REG_AT42QT_SLIDER_POS, &temp_byte);
                    
            if (temp_byte < 0x3F)
            {
                switchOffTopRightWheelLed();
                switchOnTopLeftWheelLed();
                switchOnBotLeftWheelLed();
                switchOnBotRightWheelLed();
            }
            else if (temp_byte < 0x7F)
            {
                switchOffBotRightWheelLed();
                switchOnTopLeftWheelLed();
                switchOnTopRightWheelLed();
                switchOnBotLeftWheelLed();
            }
            else if (temp_byte < 0xBF)
            {
                switchOffBotLeftWheelLed();
                switchOnTopLeftWheelLed();
                switchOnTopRightWheelLed();
                switchOnBotRightWheelLed();
            }
            else
            {
                switchOffTopLeftWheelLed();
                switchOnTopRightWheelLed();
                switchOnBotLeftWheelLed();
                switchOnBotRightWheelLed();
            }
            return_val |= RETURN_WHEEL_PRESSED;
        }
        else
        {
            switchOnTopLeftWheelLed();
            switchOnTopRightWheelLed();
            switchOnBotLeftWheelLed();
            switchOnBotRightWheelLed();
            return_val |= RETURN_WHEEL_RELEASED;
        }
                
        if (isButtonTouched() == RETURN_OK)
        {
            if (getTouchedButton() == LEFT_BUTTON)
            {
                switchOffLeftButonLed();
                return_val |= RETURN_LEFT_PRESSED;
                return_val |= RETURN_RIGHT_RELEASED;
            }
            else if(getTouchedButton() == RIGHT_BUTTON)
            {
                switchOffRightButonLed();
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
            switchOnLeftButonLed();
            switchOnRightButonLed();
            return_val |= RETURN_PROX_RELEASED;
            return_val |= RETURN_LEFT_RELEASED;
            return_val |= RETURN_RIGHT_RELEASED;
        }
    }
    
    return return_val;   
}