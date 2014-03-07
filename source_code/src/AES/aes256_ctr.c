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
/*!	\file 	aes256_ctr.c
*	\brief	AES256CTR encryption
* 
*	Created: 06/03/2014 14:17:00
*	Author: Miguel A. Borrego
*/

#include "aes.h"
#include <stdint.h>
#include <string.h>

/*!	\fn 	static void xor(uint8_t* dest, uint8_t* src, uint8_t nbytes)
*	\brief	Do xor between dest and src and save it inside dest
* 
*   \param  uint8_t *dest - destination of xor
*   \param  uint8_t *src - source of xor data
*   \param  uint8_t nbytes - number of bytes to be xored between dest and src
*/
static void xor(uint8_t *dest, uint8_t *src, uint8_t nbytes)
{
	while(nbytes--)
	{
		*dest ^= *src;
		dest++;
		src++;
	}
}

/*!	\fn 	void aes256CtrEnc(const void *iv, const void *key, void *text)
*	\brief	Encrypt a 16 byte block using CTR encryption
* 
*   \param  const void *iv - pointer to initialization vector, size 
*           must be 16 bytes
*   \param  const void *key - pointer to key, size must be 32 bytes
*   \param  void *text - plainText to be encrypted, size must be 16 bytes.
*/
void aes256CtrEnc(const void *iv, const void *key, void *text)
{
    // the context where the round keys are stored
    aes256_ctx_t ctx;
	
	// copy iv to ivcopy
	uint8_t ivcopy[16];
	
	memcpy(ivcopy, iv, 16);
	
	// init aes256
	aes256_init(key, &ctx);

	// encrypt ivcopy and key, then store the result in ivcopy
	aes256_enc(ivcopy, &ctx);
	
	// xor encoded ivcopy with text
	xor(text, ivcopy, 16);
}

/*!	\fn 	void aes256CtrDec(const void *iv, const void *key, void *text)
*	\brief	Decrypt a 16 byte block using CTR encryption
* 
*   \param  const void *iv - pointer to initialization vector, size 
*           must be 16 bytes
*   \param  const void *key - pointer to key, size must be 32 bytes
*   \param  void *text - cipherText to be decrypted, size must be 16 bytes.
*/
void aes256CtrDec(const void *iv, const void *key, void *text)
{
	// Decrypt is the same operation as encrypt
	aes256CtrEnc(iv, key, text);
}
