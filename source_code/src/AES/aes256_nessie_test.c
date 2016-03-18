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
/*!	\file 	aes256_nessie_test.c
*	\brief	Different functions to check AES256 using nessie test vectors. \n
*           see https://www.cosic.esat.kuleuven.be/nessie/testvectors/ \n
*           Block Cipher -> Rijndael -> key size 256. \n 
*           To verify the output of tests, do a log with output 
*           and see differences with a diff viewer. \n
* 
*	Created: 16/02/2014 13:54:34
*	Author: Miguel A. Borrego
*/

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "aes256_nessie_test.h"
#include "aes.h"
#include "utils.h"
#ifdef NESSIE_TEST_VECTORS

/*! \var int8_t (*nessieOutput)(uint8_t ch)
 *  \brief function pointer to the output function 
 */
int8_t (*nessieOutput)(uint8_t ch);

/*!	\fn 	static void printChar(char data)
*	\brief	Print a char to the passing the value to nessieOutput function ptr.
* 
*   \param  data - char to be printed
*/
static void printChar(char data)
{
    if(nessieOutput != 0)
    {
        nessieOutput(data);
    }
}

/*!	\fn 	static void printString(char* data)
*	\brief	Print a string
* 
*   \param  data - pointer to string
*/
static void printString(char* data)
{
    while(*data != 0)
    {
        printChar(*data++);
    }
}

/*!	\fn 	static void printStringP(const char *data)
*	\brief	Print a string using FLASH stored data
* 
*   \param  data - pointer to flash string
*/
static void printStringP(const char *data)
{
    char c;
    while((c = pgm_read_byte(data++)))
    {
        printChar(c);
    }
}

/*!	\fn 	static void printCharTimes(char c, uint8_t times)
*	\brief	Print a n times.\n
*           For example: printCharTimes('1', 5) would print '11111'.
* 
*   \param  c - The ascii char
*   \param  times - The number of times to be printed
*/
static void printCharTimes(char c, uint8_t times)
{
    while(times--)
    {
        printChar(c);
    }
}

/*!	\fn 	static void printHex(uint8_t *ptr, uint8_t size)
*	\brief	Print hexadecimal representation of an array of uint8_t.
* 
*   \param  ptr - Pointer to array of uint8_t
*   \param  size - Size of array
*/
static void printHex(uint8_t *ptr, uint8_t size)
{
    uint8_t i;
    char str[3];
    
    for(i=0; i<size; i++)
    {
        hexachar_to_string(ptr[i], str);
        printString(str);
    }
}

/*!	\fn 	static void printUint8(uint8_t num)
*	\brief	Print decimal representation of uint8_t.
* 
*   \param  num - The number to print
*/
static void printUint8(uint8_t num)
{
    // uint8_t to string
    char str[4]; // 3ch + '\0'
    char_to_string((unsigned char)num, str);
    printString(str);
}

/*!	\fn 	static void printTestHeader(uint8_t num)
*	\brief	Print header of nessie test vectors, example calling
*           printTestHeader(1):\n
*
*           'Test vectors -- set 1'
*           '====================='
* 
*   \param  num - The number to print after 'set' word
*/
static void printTestHeader(uint8_t num)
{
    printStringP(PSTR("Test vectors -- set "));
    printUint8(num);
    printStringP(PSTR("\n=====================\n\n"));
}

/*!	\fn 	static void printTestVectorHeader(uint8_t num1, uint8_t num2)
*	\brief	Print set and vector number of each vector generated, 
*           example calling printTestVectorHeader(1, 0):\n
*
*           'Set 1, vector#  0:'
* 
*   \param  num1 - The Set number
*   \param  num2 - The Vector number
*/
static void printTestVectorHeader(uint8_t num1, uint8_t num2)
{
    printStringP(PSTR("Set "));
    printUint8(num1);
    printStringP(PSTR(", vector#"));
    
    // Place a space in hundreds or tens position if num2 hundreds or tens
    // value are 0.
    if(num2<100)
    {
        printChar(' ');
        if(num2 < 10)
        {
            printChar(' ');
        }
    }
    printUint8(num2);
    printStringP(PSTR(":\n"));
}

/*!	\fn 	static void printTestKey(uint8_t* key)
*	\brief	Print key string and key value in hexa, example calling
*           printTestKey(key) with key initializated to 0:
*
*           'key=00000000000000000000000000000000
*            00000000000000000000000000000000'
* 
*   \param  key - Pointer to key array
*/
static void printTestKey(uint8_t *key)
{
    // Print the Key in hex
    printCharTimes(' ', 27);
    printStringP(PSTR("key="));
    printHex(key,16);
    printChar('\n');
    printCharTimes(' ', 31);
    printHex(&key[16],16);
    printChar('\n'); 
}

/*!	\fn 	static void printTestPlain(uint8_t *data)
*	\brief	Print plain string and data value in hexa, example calling
*           printTestPlain(data) with data initialized to 0:\n
*
*           'plain=00000000000000000000000000000000'
* 
*   \param  data - Pointer to data array
*/
static void printTestPlain(uint8_t *data)
{
    // Print plain in hex
    printCharTimes(' ', 25);
    printStringP(PSTR("plain="));
    printHex(data,16);
    printChar('\n');
}

/*!	\fn 	static void printTestCipher(uint8_t *cipher)
*	\brief	Print cipher string and cipher value in hexa, example calling
*           printTestCipher(cipher):\n
*
*           'cipher=E35A6DCB19B201A01EBCFA8AA22B5759'
* 
*   \param  cipher - Pointer to cipher array
*/
static void printTestCipher(uint8_t *cipher)
{
    // Print cipher in hex
    printCharTimes(' ', 24);
    printStringP(PSTR("cipher="));
    printHex(cipher,16);
    printChar('\n');
}

/*!	\fn 	static void printTestDecrypted(uint8_t *decrypted)
*	\brief	Print decrypted string and decrypted value in hexa, example calling
*           printTestDecrypted(decrypted):\n
*
*           'decrypted=00000000000000000000000000000000'
* 
*   \param  decrypted - Pointer to decrypted array
*/
static void printTestDecrypted(uint8_t *decrypted)
{
    // Print decrypted in hex
    printCharTimes(' ', 21);
    printStringP(PSTR("decrypted="));
    printHex(decrypted,16);
    printChar('\n');
}

/*!	\fn 	static void printTestEncrypted(uint8_t *encrypted)
*	\brief	Print encrypted string and encrypted value in hexa, example calling
*           printTestEncrypted(encrypted):\n
*
*           'encrypted=E35A6DCB19B201A01EBCFA8AA22B5759'
* 
*   \param  encrypted - Pointer to encrypted array
*/
static void printTestEncrypted(uint8_t *encrypted)
{
    // Print encrypted in hex
    printCharTimes(' ', 21);
    printStringP(PSTR("encrypted="));
    printHex(encrypted,16);
    printChar('\n');
}

/*!	\fn 	static void printTest100Times(uint8_t *data)
*	\brief	Print Iterated100times string and data value in hexa, 
*           example calling printTest100Times(data):\n
*
*           'Iterated100times==E35A6DCB19B201A01EBCFA8AA22B5759'
* 
*   \param  data - Pointer to data array
*/
static void printTest100Times(uint8_t *data)
{
    printCharTimes(' ', 12);
    printStringP(PSTR("Iterated 100 times="));		
    printHex(data,16);
    printChar('\n');
}

/*!	\fn 	static void printTest1000Times(uint8_t *data)
*	\brief	Print Iterated1000times string and data value in hexa, 
*           example calling printTest1000Times(data):\n
*
*           'Iterated1000times==E35A6DCB19B201A01EBCFA8AA22B5759'
* 
*   \param  data - Pointer to data array
*/
static void printTest1000Times(uint8_t *data)
{
    printCharTimes(' ', 11);
    printStringP(PSTR("Iterated 1000 times="));		
    printHex(data,16);
    printChar('\n');
}

/*!	\fn 	static void GenerateOutput1to4(uint8_t *key, uint8_t *data)
*	\brief	Generate output from Nessie test 1 to 4.
* 
*   \param  key - Pointer to key array
*   \param  data - Pointer to data array
*/
static void GenerateOutput1to4(uint8_t *key, uint8_t *data)
{
    // declare counter
    uint16_t j;
    
    // the context where the round keys are stored
    aes256_ctx_t ctx;
    
    // Print the Key
    printTestKey(key);
    
    // init aes with the key to be used
    aes256_init(key, &ctx);

    // Print plain in hex
    printTestPlain(data);
    
    // Print first encoded value
    aes256_enc(data, &ctx);
    printTestCipher(data);

    // Print first decoded value
    aes256_dec(data, &ctx);
    printTestDecrypted(data);

    // print 100 times and 1000 times
    for (j=0; j<1000; j++)
    {
        aes256_enc(data, &ctx);
        if(j==99)
        {
            printTest100Times(data);
        }
        else if (j==999)
        {
            printTest1000Times(data);
        }
    }
    
    // Add newline
    printChar('\n');
}

/*!	\fn 	static void GenerateOutput5to8(uint8_t *key, uint8_t *data)
*	\brief	Generate output from Nessie test 5 to 8
* 
*   \param  key - Pointer to key array
*   \param  data - Pointer to data array
*/
static void GenerateOutput5to8(uint8_t *key, uint8_t *data)
{
    // the context where the round keys are stored
    aes256_ctx_t ctx;
    
    // Print the Key
    printTestKey(key);

    // init aes with the key to be used
    aes256_init(key, &ctx);
    aes256_dec(data, &ctx);
    
    // Print first encoded value
    aes256_enc(data, &ctx);
    printTestCipher(data);

    // Print first decoded value
    aes256_dec(data, &ctx);
    printTestPlain(data);

    // Print first decoded value
    aes256_enc(data, &ctx);
    printTestEncrypted(data);
    
    // newline
    printChar('\n');
}

/*!	\fn 	static void arraySet(uint8_t *array, uint8_t value, uint8_t size)
*	\brief	The same function as memset created to avoid including standard
*           libraries.
* 
*   \param  array - pointer to the array to set
*   \param  value - value to load into each array byte
*   \param  size - size of array
*/
static void arraySet(uint8_t *array, uint8_t value, uint8_t size)
{
    uint8_t *p = array;
    
    while(size--)
    {
        *p++ = value;
    }
}

/*!	\fn 	void nessieTest(uint8_t setnum)
*	\brief	Perform desired nessie test. Test from 1 to 8
* 
*   \param  setnum - Number of test from 1 to 8
*/
void nessieTest(uint8_t setnum)
{
    // declare counters
    uint16_t i,j;
    
    // declare AES256 key and data
    uint8_t key[32];
    uint8_t data[16];

    // print the number of test to be done
    printTestHeader(setnum);
    
    // declare number of loops to be done
    // for the given nessie test in setnum
    uint16_t maxiter;
    
    // Set the max number of loops to be done
    if (setnum==1 || setnum==5)
    {
        maxiter = 256;
    }
    else if(setnum==2 || setnum==6)
    {
        maxiter = 128;
    }
    else if(setnum==3 || setnum==7)
    {
        maxiter = 256;
    }
    else if(setnum==4 || setnum==8)
    {
        maxiter = 2;
    }
    else
    {
        return;
    }
    
    for(i=0; i<maxiter; i++)
    {
        // print current test set and vector
        printTestVectorHeader(setnum, (uint8_t)i);
        
        // Reset key
        arraySet(key, 0, 32);

        // Reset data
        arraySet(data, 0, 16);
        
        // Switch to setup the key and data 
        switch(setnum)
        {
            /* To perform test 1 or 5 we shift the Most
            *  Significative Bit of the key to right (256 times).
            */
            case 1:
            case 5:
                // set key
                key[i/8] = (0x80 >> (i%8));
                // don't change data, it should be 0
                break;
                
            /* To perform test 2 we shift the Most
            *  Significative Bit of the Plain text to right.
            */
            case 2:
            case 6:
                // don't change key, it should be 0
                // set data
                data[i/8] = (0x80 >> (i%8));
                break;
                
            /* To perform test 3 we load 
            *  the iteration value to key and data. If we are on the second
            *  iteration of the loop, key and data should contain:
            *
            *  key=02020202020202020202020202020202
            *  02020202020202020202020202020202
            *  plain=02020202020202020202020202020202
            */
            case 3:
            case 7:
                // set key
                arraySet(key, i, 32);
                // set data
                arraySet(data, i, 16);
                break;
            
            /* Test-4 just test two pair of key-data vectors. */
            case 4:
            case 8:
                if (i==0)
                {
                    // set key
                    for (j=0; j<32; j++)
                    {
                        key[j]=j;
                    }

                    // set data
                    for (j=0; j<16; j++)
                    {
                        data[j]= (j<<4) + j;
                    }
                }
                // key and data hardcoded
                else if(i==1)
                {
                    // set key
                    key[0] = 0x2B;
                    key[1] = 0xD6;
                    key[2] = 0x45;
                    key[3] = 0x9F;
                    key[4] = 0x82;
                    key[5] = 0xC5;
                    key[6] = 0xB3;
                    key[7] = 0x00;
                    key[8] = 0x95;
                    key[9] = 0x2C;
                    key[10] = 0x49;
                    key[11] = 0x10;
                    key[12] = 0x48;
                    key[13] = 0x81;
                    key[14] = 0xFF;
                    key[15] = 0x48;
                    
                    for (j=0; j<16; j++)
                    {
                        key[j+16] = key[j];
                    }

                    // set data
                    data[0] = 0xEA;
                    data[1] = 0x02;
                    data[2] = 0x47;
                    data[3] = 0x14;
                    data[4] = 0xAD;
                    data[5] = 0x5C;
                    data[6] = 0x4D;
                    data[7] = 0x84;
                    data[8] = 0xEA;
                    data[9] = 0x02;
                    data[10] = 0x47;
                    data[11] = 0x14; 
                    data[12] = 0xAD; 
                    data[13] = 0x5C;
                    data[14] = 0x4D;
                    data[15] = 0x84;
                }
                break;
                
            /* default case, return from the function call */
            default:
                return;
                
        } /* end switch */
        
        // Output is different for set1to4 and set5to8
        if(setnum<5)
        {
            // Do test_1to4
            GenerateOutput1to4(key, data);
        }
        else
        {
            // Do test_5to8
            GenerateOutput5to8(key, data);
        }
        
    } /* end for loop */
    
} /* end nessieTest call */
#endif