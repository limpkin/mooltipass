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
/*! \file   touch.h
 *  \brief  Touch sensing functions
 *  Copyright [2014] [Mathieu Stephan]
 */


#ifndef TOUCH_H_
#define TOUCH_H_

RET_TYPE readDataFromTS(uint8_t addr, uint8_t reg, uint8_t* data);
RET_TYPE writeDataToTS(uint8_t addr, uint8_t reg, uint8_t data);
RET_TYPE initTouchSensing(void);
void initI2cPort(void);

/*! \fn     start_condition()
*   \brief  Generate start condition
*/
#define start_condition()       (TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN))

/*! \fn     stop_condition()
*   \brief  Generate stop condition
*/
#define stop_condition()        (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO))

/*! \fn     clear_twint_flag()
*   \brief  Clear TWINT flag
*/
#define clear_twint_flag()      (TWCR = (1<<TWINT) | (1<<TWEN))

/*! \fn     acknowledge_data()
*   \brief  Acknowledge received data, ask for a new one
*/
#define acknowledge_data()      (TWCR = (1<<TWINT) | (1<<TWEN) | (1 << TWEA))

#endif /* TOUCH_H_ */