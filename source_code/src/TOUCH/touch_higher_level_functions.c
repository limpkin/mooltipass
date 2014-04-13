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
#include "touch.h"


/*! \fn     checkTSPres()
*   \brief  Check that the AT42QT2120 is here
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
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY0_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY1_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY2_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY3_CTRL, AT42QT2120_GUARD_VAL|AT42QT2120_AKS_GP1_MASK);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY4_CTRL, AT42QT2120_OUTPUT_L_VAL);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY5_CTRL, AT42QT2120_OUTPUT_L_VAL);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY6_CTRL, AT42QT2120_OUTPUT_L_VAL);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY7_CTRL, AT42QT2120_OUTPUT_L_VAL);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY8_CTRL, AT42QT2120_OUTPUT_L_VAL);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY9_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY10_CTRL, AT42QT2120_OUTPUT_L_VAL);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_KEY11_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);
            // Enable wheel
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_SLID_OPT, 0x40);
            writeDataToTS(AT42QT2120_ADDR, REG_AT42QT_SLID_OPT, 0xC0);
        } 
        
        return temp_return;
    #else
        return RETURN_NOK;
    #endif
}