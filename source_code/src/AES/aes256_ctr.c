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
#include "aes256_ctr.h"

/*!	\fn 	static void xor(uint8_t* dest, uint8_t* src, uint8_t nbytes)
*	\brief	Do xor between dest and src and save it inside dest
* 
*   \param  dest - destination of xor
*   \param  src - source of xor data
*   \param  nbytes - number of bytes to be xored between dest and src
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

/*!	\fn 	void aes256CtrEncBlock(const void *iv, const void *key, void *text)
*	\brief	Encrypt a 16 byte block using CTR encryption
* 
*   \param  iv - pointer to initialization vector, size 
*           must be 16 bytes
*   \param  key - pointer to key, size must be 32 bytes
*   \param  text - plainText to be encrypted, size must be 16 bytes.
*/
void aes256CtrEncBlock(const void *iv, const void *key, void *text)
{
    // the context where the round keys are stored
    aes256_ctx_t ctx;

    // copy iv to ivcopy
    uint8_t ivcopy[16];

    // temp var
    uint8_t i;
    uint8_t *ptr;

    ptr = (uint8_t*)iv;

    for(i = 0; i < 16; i++)
    {
        ivcopy[i] = *ptr++;
    }

    // init aes256
    aes256_init(key, &ctx);

    // encrypt ivcopy with key, then store the result in ivcopy
    aes256_enc(ivcopy, &ctx);

    // xor encoded ivcopy with text
    xor(text, ivcopy, 16);

    for(i = 0; i < 16; i++)
    {
        ivcopy[i] = 0x00;
    }
}

/*!	\fn 	void aes256CtrDecBlock(const void *iv, const void *key, void *text)
*	\brief	Decrypt a 16 byte block using CTR encryption
* 
*   \param  iv - pointer to initialization vector, size 
*           must be 16 bytes
*   \param  key - pointer to key, size must be 32 bytes
*   \param  text - cipherText to be decrypted, size must be 16 bytes.
*/
void aes256CtrDecBlock(const void *iv, const void *key, void *text)
{
    // Decrypt is the same operation as encrypt
    aes256CtrEncBlock(iv, key, text);
}

/*!	\fn 	void aes256CtrInit(aes256CtrCtx *ctx, const uint8_t *key, const uint8_t *iv, size_t ivLen)
*	\brief	Init CTR encryption and save key and iv inside ctx
* 
*   \param  ctx - context to save iv and key information
*   \param  iv - pointer to initialization vector, size must be 16 bytes or lower.
*   \param  key - pointer to key, size must be 32 bytes
*/
void aes256CtrInit(aes256CtrCtx_t *ctx, const uint8_t *key, const uint8_t *iv, uint8_t ivLen)
{
	uint8_t i;

	// copy key inside CTX
	for (i=0; i<32; i++)
	{
		ctx->key[i] = key[i];
	}

	// ivLen must be 16 or lower
	if (ivLen > 16)
	{
		return;
	}

	// copy iv inside CTX
	for (i=0; i<ivLen; i++)
	{
		ctx->ctr[i] = iv[i];
	}

	// zero rest of bytes of ctx->iv.
	for (i=ivLen; i<16; i++)
	{
		ctx->ctr[i] = 0x00;
	}
}

/*!	\fn 	void incrementCtr(uint8_t *ctr)
*	\brief	Increment ctr by 1
* 
*   \param  ctr - pointer to counter+iv, size must be 16 bytes.
*/
void incrementCtr(uint8_t *ctr)
{
    uint8_t i;

    i = 15;
    while (ctr[i]++ == 0xFF) 
    {
        if (i == 0)
        {
            break;
        }

        i--;
    }
}

/*!	\fn 	aes256CtrEncrypt(aes256CtrCtx_t *ctx, uint8_t *data, uint16_t dataLen)
*	\brief	Encrypt data and save it in data.
* 
*   \param  data - pointer to data, this is also the location to store encrypted data
*   \param  dataLen - size of data, must be multiple of 16.
*/
void aes256CtrEncrypt(aes256CtrCtx_t *ctx, uint8_t *data, uint16_t dataLen)
{
    uint16_t i;

    // dataLen must be multiple of 16 !
    if(dataLen%16 != 0)
    {
        return;
    }

    // start encryption/decryption
    for (i=0; i<dataLen; i+=16)
    {
        aes256CtrEncBlock(ctx->ctr, ctx->key, &data[i]);
        incrementCtr(ctx->ctr);
    }
}

/*!	\fn 	aes256CtrDecrypt(aes256CtrCtx_t *ctx, uint8_t *data, uint16_t dataLen)
*	\brief	Decrypt data and save it in data.
* 
*   \param  data - pointer to data, this is also the location to store encrypted data
*   \param  dataLen - size of data, must be multiple of 16.
*/
void aes256CtrDecrypt(aes256CtrCtx_t *ctx, uint8_t *data, uint16_t dataLen)
{
	aes256CtrEncrypt(ctx, data, dataLen);
}
