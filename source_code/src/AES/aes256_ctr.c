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

/*!	\fn 	void aesXorVectors(uint8_t* dest, uint8_t* src, uint8_t nbytes)
*	\brief	Do xor between dest and src and save it inside dest
*
*   \param  dest - destination of xor
*   \param  src - source of xor data
*   \param  nbytes - number of bytes to be xored between dest and src
*/
void aesXorVectors(uint8_t *dest, const uint8_t *src, uint8_t nbytes)
{
    while (nbytes--)
    {
        *dest ^= *src;
        dest++;
        src++;
    }
}

/*!	\fn 	void aes256CtrInit(aes256CtrCtx_t *ctx, const uint8_t *key, const uint8_t *iv, uint8_t ivLen)
*	\brief	Init CTR encryption and save key and iv inside ctx
*
*   \param  ctx - context
*   \param  key - pointer to key, size must be 32 bytes
*   \param  iv - pointer to initialization vector, must be 16 or lower.
*   \param  ivLen - length of initialization vector, must be 16 or lower.
*/
void aes256CtrInit(aes256CtrCtx_t *ctx, const uint8_t *key, const uint8_t *iv, uint8_t ivLen)
{
	// ivLen must be 16 or lower
	if (ivLen > 16)
	{
		return;
	}

	// initialize key schedule inside CTX
	aes256_init(key, &(ctx->aesCtx));

	// initialize iv and cipherstream cache
	aes256CtrSetIv(ctx, iv, ivLen);
}

/*!	\fn 	void aes256CtrSetIv(aes256CtrCtx_t *ctx, const uint8_t *iv, uint8_t ivLen)
*	\brief	Re-Init CTR encryption without changing the key
*
*   \param  ctx - context
*   \param  iv - pointer to initialization vector, size must be 16 bytes or lower.
*   \param  ivLen - length of initialization vector, must be 16 or lower.
*/
void aes256CtrSetIv(aes256CtrCtx_t *ctx, const uint8_t *iv, uint8_t ivLen)
{
	uint8_t i;

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
	for (i = ivLen; i < 16; i++)
	{
		ctx->ctr[i] = 0x00;
	}

	// invalidate cipherstream cache
	ctx->cipherstreamAvailable = 0;
}

/*!	\fn 	void aesIncrementCtr(uint8_t *ctr, uint8_t len)
*	\brief	Increment ctr by 1
*
*   \param  ctr - pointer to counter+iv, size must be len bytes
*   \param  len - the size of the ctr buffer to be incremented
*/
void aesIncrementCtr(uint8_t *ctr, uint8_t len)
{
    uint8_t i;

    if (len == 0)
    {
        return;
    }

    i = len-1;
    while (ctr[i]++ == 0xFF)
    {
        if (i == 0)
        {
            break;
        }

        i--;
    }
}

/*! \fn     int8_t aesCtrCompare(uint8_t *ctr1, uint8_t *ctr2, uint8_t len)
*   \brief  compare ctr1 and ctr2
*
*   \param  ctr1 - buffer of first ctr
*   \param  ctr2 - buffer of second ctr
*   \param  len - length of the ctr, must be the same for ctr1 and ctr2
*/
int8_t aesCtrCompare(uint8_t *ctr1, uint8_t *ctr2, uint8_t len)
{
    // -1 ctr1 < ctr2
    // 0 same
    // 1  ctr1 > ctr2
    int8_t result = 0; // same
    uint8_t i;

    for (i = 0; i < len; i++)
    {
        if(ctr1[i] != ctr2[i])
        {
            if(ctr1[i] < ctr2[i])
            {
                result = -1;
            }
            else
            {
                result = 1;
            }

            // go outside the loop
            break;
        }
    }

    return result;
}

/*!	\fn 	aes256CtrEncrypt(aes256CtrCtx_t *ctx, uint8_t *data, uint16_t dataLen)
*	\brief	Encrypt data and save it in data.
*
*   \param  ctx - context
*   \param  data - pointer to data, this is also the location to store encrypted data
*   \param  dataLen - size of data
*/
void aes256CtrEncrypt(aes256CtrCtx_t *ctx, uint8_t *data, uint16_t dataLen)
{
    uint16_t i;

    // Loop will advance by a variable amount: ctx->cipherstreamAvailable in the
    // first round, 16 then, dataLen - i in the last round.
    for (i = 0; i < dataLen; )
    {
        // if we need new cipherstream, calculate it
        if (ctx->cipherstreamAvailable == 0)
        {
            uint8_t j;
            for(j = 0; j < 16; j++)
            {
                ctx->cipherstream[j] = ctx->ctr[j];
            }

            // encrypt ctr with key, then store the result in cipherstream
            aes256_enc(ctx->cipherstream, &(ctx->aesCtx));

            ctx->cipherstreamAvailable = 16;
        }

        uint16_t thisLoop = dataLen - i;

        // in this go we can only do at most cipherStreamAvailable bytes
        if (thisLoop > ctx->cipherstreamAvailable)
        {
            thisLoop = ctx->cipherstreamAvailable;
        }

        // do the actual encryption/decryption, update state
        aesXorVectors(data + i, ctx->cipherstream + 16 - ctx->cipherstreamAvailable, thisLoop);
        i += thisLoop;
        ctx->cipherstreamAvailable -= thisLoop;

        // if the cached cipherstream is fully used, increment ctr
        if (ctx->cipherstreamAvailable == 0)
        {
            aesIncrementCtr(ctx->ctr, 16);
        }
    }
}

/*!	\fn 	aes256CtrDecrypt(aes256CtrCtx_t *ctx, uint8_t *data, uint16_t dataLen)
*	\brief	Decrypt data and save it in data.
*
*   \param  ctx - context
*   \param  data - pointer to data, this is also the location to store decrypted data
*   \param  dataLen - size of data
*/
void aes256CtrDecrypt(aes256CtrCtx_t *ctx, uint8_t *data, uint16_t dataLen)
{
	aes256CtrEncrypt(ctx, data, dataLen);
}


/*!	\fn 	aes256CtrClean(aes256CtrCtx_t *ctx)
*	\brief	Clean the context
*
*   \param  ctx - context
*/
void aes256CtrClean(aes256CtrCtx_t *ctx)
{
	uint8_t *ptr = (uint8_t*)ctx;
	uint16_t i;
	for (i = 0; i < sizeof(*ctx); i++)
	{
		*ptr++ = 0;
	}
}
