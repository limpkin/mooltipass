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
/*! \file   touch.c
 *  \brief  Touch sensing functions
 *  Copyright [2014] [Mathieu Stephan]
 */
#include "touch_higher_level_functions.h"
#include "defines.h"
#include <avr/io.h>
#include "touch.h"


/*! \fn     waitForTwintFlag(void)
*   \brief  Wait for TWINT flag, indicating that current task is finished
*/
static inline void waitForTwintFlag(void)
{
    while(!(TWCR & (1<<TWINT)));
}

/*! \fn     isStartSendingOk(void)
*   \brief  Know if we successfully generated the start condition
*   \return RETURN_OK if everything is alright
*/
static inline RET_TYPE isStartSendingOk(void)
{
    if ((TWSR & 0xF8) == I2C_START)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     isRestartSendingOk(void)
*   \brief  Know if we successfully generated the restart condition
*   \return RETURN_OK if everything is alright
*/
static inline RET_TYPE isRestartSendingOk(void)
{
    if ((TWSR & 0xF8) == I2C_RSTART)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     isSlawSendingOk(void)
*   \brief  Know if we successfully sent the addr byte
*   \return RETURN_OK if everything is alright
*/
static inline RET_TYPE isSlawSendingOk(void)
{
    if ((TWSR & 0xF8) == I2C_SLA_ACK)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     isSlarSendingOk(void)
*   \brief  Know if we successfully sent the addr byte (reading mode)
*   \return RETURN_OK if everything is alright
*/
static inline RET_TYPE isSlarSendingOk(void)
{
    if ((TWSR & 0xF8) == I2C_SLAR_ACK)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     isDataSendingOk(void)
*   \brief  Know if we successfully sent the data byte
*   \return RETURN_OK if everything is alright
*/
static inline RET_TYPE isDataSendingOk(void)
{
    if ((TWSR & 0xF8) == I2C_DATA_ACK)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     initiateI2cWrite(uint8_t addr, uint8_t reg)
*   \brief  Initiate a write process in the at42qt2120
*   \param  addr        The chip address
*   \param  reg         The register address
*   \return RETURN_OK if everything is alright, the pb code otherwise
*/
RET_TYPE initiateI2cWrite(uint8_t addr, uint8_t reg)
{
    start_condition();
    waitForTwintFlag();
    if(isStartSendingOk() != RETURN_OK)
    {
        stop_condition();
        return I2C_START_ERROR;
    }

    TWDR = addr;
    clear_twint_flag();
    waitForTwintFlag();
    if(isSlawSendingOk() != RETURN_OK)
    {
        stop_condition();
        return I2C_SLA_ERROR;
    }

    TWDR = reg;
    clear_twint_flag();
    waitForTwintFlag();
    if(isDataSendingOk() != RETURN_OK)
    {
        stop_condition();
        return I2C_DATA_ERROR;
    }

    return RETURN_OK;
}

/*! \fn     writeDataToTS(uint8_t reg, uint8_t data)
*   \brief  Write a byte inside the AT42QT2120
*   \param  addr        The chip address
*   \param  reg         The register address
*   \param  data        The data to write
*   \return RETURN_OK if everything is alright, the pb code otherwise
*/
RET_TYPE writeDataToTS(uint8_t reg, uint8_t data)
{
    RET_TYPE ret_val;

    ret_val = initiateI2cWrite(AT42QT2120_ADDR, reg);
    if(ret_val != RETURN_OK)
    {
        return ret_val;
    }

    TWDR = data;
    clear_twint_flag();
    waitForTwintFlag();
    if(isDataSendingOk() != RETURN_OK)
    {
        stop_condition();
        return I2C_DATA_ERROR;
    }

    stop_condition();
    return RETURN_OK;
}

/*! \fn     initiateI2cRead(uint8_t addr, uint8_t reg)
*   \brief  Initiate a read process in the at42qt2120
*   \param  addr        The chip address
*   \param  reg         The register address
*   \return RETURN_OK if everything is alright, the pb code otherwise
*/
RET_TYPE initiateI2cRead(uint8_t addr, uint8_t reg)
{
    RET_TYPE ret_val;

    ret_val = initiateI2cWrite(addr, reg);
    if(ret_val != RETURN_OK)
    {
        return ret_val;
    }

    start_condition();
    waitForTwintFlag();
    if(isRestartSendingOk() != RETURN_OK)
    {
        stop_condition();
        return I2C_RSTART_ERR;
    }

    TWDR = addr | 0x01;
    clear_twint_flag();
    waitForTwintFlag();
    if(isSlarSendingOk() != RETURN_OK)
    {
        stop_condition();
        return I2C_SLAR_ERROR;
    }

    return RETURN_OK;
}

/*! \fn     readDataFromTS(uint8_t reg, uint8_t* data)
*   \brief  Write a byte inside the AT42QT2120
*   \param  reg         The register address
*   \param  data        uint8_t pointer in which we write the data
*   \return RETURN_OK if everything is alright, the pb code otherwise
*/
RET_TYPE readDataFromTS(uint8_t reg, uint8_t* data)
{
    RET_TYPE ret_val;

    ret_val = initiateI2cRead(AT42QT2120_ADDR, reg);
    if(ret_val != RETURN_OK)
    {
        return ret_val;
    }

    clear_twint_flag();
    waitForTwintFlag();
    *data = TWDR;
    stop_condition();

    return RETURN_OK;
}

/*! \fn     initI2cPort()
*   \brief  Initialize ports & i2c controller
*/
void initI2cPort(void)
{
    #ifndef HARDWARE_V1
        PORT_TOUCH_C |= (1 << PORTID_TOUCH_C);  // Touch change pin as input with pullup
        PORT_I2C_SCL |= (1 << PORTID_I2C_SCL);  // Set I2C ports as output & high
        PORT_I2C_SDA |= (1 << PORTID_I2C_SDA);  // Set I2C ports as output & high
        DDR_TOUCH_C &= ~(1 << PORTID_TOUCH_C);  // Touch change pin as input with pullup
        DDR_I2C_SCL |= (1 << PORTID_I2C_SCL);   // Set I2C ports as output & high
        DDR_I2C_SDA |= (1 << PORTID_I2C_SDA);   // Set I2C ports as output & high
        TWBR = 3;                               // I²C freq = 16Mhz / (16 + 2*TWBR*4^TWPS) = 400KHz
        clear_twint_flag();                     // Init I²C controller
    #endif
}