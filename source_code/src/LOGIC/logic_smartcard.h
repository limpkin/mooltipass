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
/*!  \file     logic_smartcard.h
*    \brief    Firmware logic - smartcard related tasks
*    Created:  18/08/2014
*    Author:   Mathieu Stephan
*/



#ifndef LOGIC_SMARTCARD_H_
#define LOGIC_SMARTCARD_H_

#include "defines.h"

RET_TYPE cloneSmartCardProcess(uint16_t pincode);
RET_TYPE validCardDetectedFunction(void);
RET_TYPE handleSmartcardInserted(void);
RET_TYPE removeCardAndReAuthUser(void);
void handleSmartcardRemoved(void);


#endif /* LOGIC_SMARTCARD_H_ */