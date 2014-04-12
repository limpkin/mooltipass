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

/*!  \file     node_mgmt.c
*    \brief    Mooltipass Node Management Library
*    Created:  03/4/2014
*    Author:   Michael Neiderhauser
*/

#include "../mooltipass.h"
#include "flash_mem.h"
#include "node_mgmt.h"

RET_TYPE obtainNodeManagementHandle(mgmtHandle **h){}
RET_TYPE scanNextFreeParentNode(mgmtHandle *h, uint16_t starting_address){}
RET_TYPE createParentNode(mgmtHandle *h, pNode * p){}
RET_TYPE readParentNode(mgmtHandle *h, pNode * p, uint16_t parentNodeAddress){}
RET_TYPE updateParentNode(mgmtHandle *h, pNode *p, uint16_t parentNodeAddress){}
RET_TYPE deleteParentNode(mgmtHandle *h, uint16_t parentNodeaddress, deletePolicy policy){}