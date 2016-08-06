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
/*!  \file     mini_gui_credentials_functions.h
*    \brief    General user interface - credentials functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/


#ifndef MINI_GUI_CREDENTIALS_FUNCTIONS_H_
#define MINI_GUI_CREDENTIALS_FUNCTIONS_H_

#include "node_mgmt.h"
#include "defines.h"

#define SEARCHTEXT_MAX_LENGTH   4

uint16_t guiAskForLoginSelect(pNode* p, cNode* c, uint16_t parentNodeAddress, uint8_t bypass_confirmation);
uint16_t favoriteSelectionScreen(pNode* p, cNode* c);
uint16_t loginSelectionScreen(void);

#ifdef ENABLE_CREDENTIAL_MANAGEMENT
/* On-device credential management defines, used by menus and logic */

#define ONDEVICE_CRED_MGMT_ACTION_NB 4                /* number of credential management actions */

#define ONDEVICE_CRED_MGMT_ACTION_CREATE 0            /* create new credentials and services */
#define ONDEVICE_CRED_MGMT_ACTION_EDIT   1            /* edit credentials: edit login + password charset */
#define ONDEVICE_CRED_MGMT_ACTION_RENEW  2            /* renew password */
#define ONDEVICE_CRED_MGMT_ACTION_DELETE 3            /* delete credentials */
#define ONDEVICE_CRED_MGMT_ACTION_NONE   UINT8_MAX    /* invalid action */

uint8_t managementActionSelectionScreen(void);
#endif

#endif /* MINI_GUI_CREDENTIALS_FUNCTIONS_H_ */