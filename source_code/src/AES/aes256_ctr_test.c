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
/*!	\file 	aes256_ctr_test.c
*	\brief	Different functions to check AES256CTR encryption
* 
*	Created: 06/03/2014 19:05:00
*	Author: Miguel A. Borrego
*/

#include <avr/pgmspace.h>
#include "interrupts.h"
#include "aes256_ctr.h"
#include "utils.h"
#include "usb.h"
#include "aes.h"

// Test vector, key and iv
static uint8_t v1[16] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 
0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };

static uint8_t v2[16] = { 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 
0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51 };

static uint8_t v3[16] = { 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 
0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef };

static uint8_t v4[16] = { 0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 
0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };

static uint8_t key[32] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b,
0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b, 
0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };

static uint8_t iv[16] = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 
0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };

/*! \var int8_t (*ctrTestOutput)(uint8_t c)
 *  \brief function pointer to the output function 
 */
int8_t (*ctrTestOutput)(uint8_t c) = 0;

/*!	\fn 	static void printChar(char data)
*	\brief	Print a char using ctrTestOutput function pointer
* 
*   \param  data - The char to be printed
*/
static void printChar(char data)
{
    if(ctrTestOutput != 0)
    {
        ctrTestOutput(data);
    }
}

/*!	\fn 	static void printText(char *ptr)
*	\brief	Print a string
* 
*   \param  ptr - The pointer to string
*/
static void printText(char *ptr)
{
    while(*ptr != 0)
    {
            printChar(*ptr++);
    }
}

/*!	\fn 	static void printTextP(const char *data)
*	\brief	Print a string using FLASH stored data
* 
*   \param  data - pointer to flash string
*/
static void printTextP(const char *data)
{
    char c;
    while((c = pgm_read_byte(data++)))
    {
        printChar(c);
    }
}

/*!	\fn 	static void printHex(uint8_t *ptr, uint8_t size)
*	\brief	Print hexadecimal representation of an array of uint8_t.
* 
*   \param  uint8_t *ptr - The pointer to the array of uint8_t
*   \param  uint8_t size - The size of the array
*/
static void printHex(uint8_t *ptr, uint8_t size)
{
    uint8_t i;
    char str[3];
    
    for(i=0; i<size; i++)
    {
        hexachar_to_string(ptr[i], str);
        printText(str);
    }
}

/*!	\fn 	static void printCipherText(uint8_t *data)
*	\brief	Print text 'Ciphertext' followed by hexa value of data
* 
*   \param  uint8_t *data - pointer to cipheredtext
*/
static void printCipherText(uint8_t *data)
{
    printTextP(PSTR("\nCiphertext     "));
    printHex(data, 16);
}

/*!	\fn 	static void printPlainText(uint8_t *data)
*	\brief	Print text 'Plaintext' followed by hexa value of data
* 
*   \param  uint8_t *data - pointer to plaintext
*/
static void printPlainText(uint8_t *data)
{
    printTextP(PSTR("\nPlaintext      "));
    printHex(data, 16);
}

/*!	\fn 	static void printInputBlock(uint8_t *ivector)
*	\brief	Print text 'Input Block' followed by hexa value of ivector
* 
*   \param  uint8_t *ivector - pointer to initialization vector
*/
static void printInputBlock(uint8_t *ivector)
{
    printTextP(PSTR("\nInput Block    "));
    printHex(ivector, 16);
}

/*!	\fn 	static void printKey(uint8_t *data)
*	\brief	Print text 'Key' followed by hexa value of key
* 
*   \param  uint8_t *data - pointer to the key to be printed
*/
static void printKey(uint8_t *data)
{
    printTextP(PSTR("\nKey            "));
    printHex(data, 16);
    printTextP(PSTR("\n               "));
    printHex(&data[16],16);
}

/*!	\fn 	static void printBlock(uint8_t num)
*	\brief	Print text 'Block' followed by blocknumber
* 
*   \param  uint8_t num - number to be printed next to 'Block #' string
*/
static void printBlock(uint8_t num)
{
    char str[4];
    printTextP(PSTR("\nBlock #"));
    char_to_string(num, str);
    printText(str);
}

/*!	\fn 	static void printDecryptTest(aes256CtrCtx_t *ctx, 
*                                        uint8_t *plaintext, 
*                                        uint16_t size)
*	\brief	Print the text of decryption
* 
*   \param  ctx - pointer to context
*   \param  plaintext - pointer to data to be encrypted
*   \param  size - size of data to encrypt
*/
static void printEncryptTest(aes256CtrCtx_t *ctx, uint8_t *plaintext, uint16_t size)
{
    printInputBlock(ctx->ctr);
    printPlainText(plaintext);

    // encrypt
    aes256CtrEncrypt(ctx, plaintext, size);
    printCipherText(plaintext);
}

/*!	\fn 	static void printDecryptTest(aes256CtrCtx_t *ctx, 
*                                        uint8_t *plaintext, 
*                                        uint16_t size)
*	\brief	Print the text of decryption
* 
*   \param  ctx - pointer to context
*   \param  plaintext - pointer to data to be decrypted
*   \param  size - size of data to decrypt
*/
static void printDecryptTest(aes256CtrCtx_t *ctx, uint8_t *plaintext, uint16_t size)
{
    printInputBlock(ctx->ctr);
    printPlainText(plaintext);

    // decrypt
    aes256CtrDecrypt(ctx, plaintext, size);
    printCipherText(plaintext);
}

/*!	\fn 	void aes256CtrTest(void)
*	\brief	Perform a CTR test using test vectors found in 
*           http://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf
*           page 57
*/
void aes256CtrTest(void)
{
    // init
    aes256CtrCtx_t ctx;

    aes256CtrInit(&ctx, key, iv, 16);

    // Encrypt init string
    printTextP(PSTR("CTR-AES256Encrypt"));
    
    // Print key
    printKey(key);

    // Encrypt TEST 1
    printBlock(1);
    printEncryptTest(&ctx, v1, 16);
    
    // Encrypt TEST 2
    printBlock(2);
    printEncryptTest(&ctx, v2, 16);
    
    // Encrypt TEST 3
    printBlock(3);
    printEncryptTest(&ctx, v3, 16);

    // Encrypt TEST 4
    printBlock(4);
    printEncryptTest(&ctx, v4, 16);

    // Decrypt init string
    printTextP(PSTR("\n\nCTR-AES256Decrypt"));

    aes256CtrSetIv(&ctx, iv, 16);

    // print key
    printKey(key);

    // Encrypt TEST 1
    printBlock(1);
    printDecryptTest(&ctx, v1, 16);
    
    // Encrypt TEST 2
    printBlock(2);
    printDecryptTest(&ctx, v2, 16);
    
    // Encrypt TEST 3
    printBlock(3);
    printDecryptTest(&ctx, v3, 16);

    // Encrypt TEST 4
    printBlock(4);
    printDecryptTest(&ctx, v4, 16);
}

/*!	\fn 	uint32_t aes256CtrSpeedTest(void)
*	\brief	Do 1000 encryptions and return the time needed in ms
*
*	\return time elapsed in milliseconds
*/
uint32_t aes256CtrSpeedTest(void)
{

	uint32_t time1;

	uint16_t i;

    // init
    aes256CtrCtx_t ctx;

    aes256CtrInit(&ctx, key, iv, 16);

	time1 = millis();

	for(i=0; i<1000; i++)
	{
		aes256CtrEncrypt(&ctx, v1, 16);
	}

	return millis()-time1;
}
