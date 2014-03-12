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
/*!	\file 	aes256_ctr.h
*	\brief	AES256CTR encryption Header
* 
*	Created: 06/03/2014 14:17:00
*	Author: Miguel A. Borrego
*/

#ifndef __AES_256_CTR_H__
#define __AES_256_CTR_H__

// prototype functions
void aes256CtrEnc(const void *iv, const void *key, void *text);
void aes256CtrDec(const void *iv, const void *key, void *text);

#endif
