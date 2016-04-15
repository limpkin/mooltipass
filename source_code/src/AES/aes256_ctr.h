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

#include <stdint.h>
#include "aes.h"

/*! \struct aes256CtrCtx_t
*   \brief CTX data type
*/
typedef struct
{
	aes256_ctx_t aesCtx; /*!< aes256 context */
	uint8_t ctr[16]; /*!< the value of the counter */
	uint8_t cipherstream[16]; /*!< current ctr encryption output */
	uint8_t cipherstreamAvailable; /*!< available bytes to xor with new data bytes */
}aes256CtrCtx_t;

// USEFUL functions
void aesIncrementCtr(uint8_t *ctr, uint8_t len);
void aesXorVectors(uint8_t *dest, const uint8_t *src, uint8_t nbytes);
int8_t aesCtrCompare(uint8_t *ctr1, uint8_t *ctr2, uint8_t len);

// STREAM CTR functions
void aes256CtrInit(aes256CtrCtx_t *ctx, const uint8_t *key, const uint8_t *iv, uint8_t ivLen);
void aes256CtrSetIv(aes256CtrCtx_t *ctx, const uint8_t *iv, uint8_t ivLen);
void aes256CtrEncrypt(aes256CtrCtx_t *ctx, uint8_t *data, uint16_t dataLen);
void aes256CtrDecrypt(aes256CtrCtx_t *ctx, uint8_t *data, uint16_t dataLen);
void aes256CtrClean(aes256CtrCtx_t *ctx);

// DEFINES
#define AES256_CTR_LENGTH   16

#endif
