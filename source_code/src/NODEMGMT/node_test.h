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

/* Copyright (c) 2014, Michael Neiderhauser. All rights reserved. */

/*!  \file     flash_test.h
*    \brief    Mooltipass Node Test Functions Header
*    Created:  2/5/2014
*    Author:   Michael Neiderhauser
*/

#ifndef NODE_TEST_H_
#define NODE_TEST_H_

#include "defines.h"
#include "node_mgmt.h"
#include <stdint.h>

RET_TYPE nodeTest();
RET_TYPE nodeFlagFunctionTest();
RET_TYPE nodeAddressTest();
RET_TYPE userProfileOffsetTest();
RET_TYPE nodeInitHandle();
RET_TYPE userProfileAddressTest(mgmtHandle *h);
RET_TYPE nodeScanFreeParent(mgmtHandle *h);

#endif /* FLASH_TEST_H_ */