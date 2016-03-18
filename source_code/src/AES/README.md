Mooltipass AES description
==========================

1- AES LIBRARY
--------------
In order to avoid conflicts with the GPL license of AVR Cryptolib we have decided to change the AES Library to <a href="http://www.literatecode.com/aes256">http://www.literatecode.com/aes256</a>.

To avoid changes in the current CTR implementation we decided to do some #define and avoid changing function names and those things.

Changes made from the original library to avoid changes in other files:
```
aes.c:

1.Uncomment '#define BACK_TO_TABLES'

2.Add sbox and sboxinv to flash memory:
const uint8_t sbox[256] __attribute__ ((__progmem__)) = {...};
const uint8_t sboxinv[256] __attribute__ ((__progmem__)) = {...};

3.Modify #define of rj_sbox and rj_sbox_inv to:
#define rj_sbox(x)     (pgm_read_byte(&sbox[x]))
#define rj_sbox_inv(x) (pgm_read_byte(&sboxinv[x]))

4.Change aes_init function name to aes256_init_ecb
```
```
aes.h:

1- Add '#define' inside the header file
#define aes256_ctx_t aes256_context

#define aes256_init(x,y)	aes256_init_ecb((y),(uint8_t*)(x))
#define aes256_enc(x,y)		aes256_encrypt_ecb((y),(uint8_t*)(x))
#define aes256_dec(x,y)		aes256_decrypt_ecb((y),(uint8_t*)(x))

```

How to use the library? How to work  with it ? As easy as it sounds, you only have to care about 3 functions: aes256_init, aes256_enc and aes256_dec. Here it is a simple example:

```
void aes256_test(void)
{
    // aes256 is 32 byte key
    uint8_t key[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20, 21,22,23,24,25,26,27,28,29,30,31};

    // aes256 is 16 byte data size
    uint8_t data[16] = "text to encrypt";

    // declare the context where the round keys are stored
    aes256_ctx_t ctx;

    // Initialize the AES256 with the desired key
    aes256_init(key, &ctx);

    // Encrypt data
    // "text to encrypt" (ascii) -> '9798D10A63E4E167122C4C07AF49C3A9'(hex)
    aes256_enc(data, &ctx);

    // Decrypt data
    // '9798D10A63E4E167122C4C07AF49C3A9'(hex) -> "text to encrypt" (ascii)
    aes256_dec(data, &ctx);
}
```

2- Testing the library using nessie test vectors
------------------------------------------------
After downloading a third party library or resource you must ensure the library performs the function as well as it is claimed. So to satisfy our paranoia against any bug or error with the library, we have checked the encryption and decryption using different test vectors, called Nessie Test Vectors. There are 8 different sets of test vectors, we have checked AES256 against all.

To test AES256 using nessie vectors, we have created a file called aes256_nessie_test.c. This file outputs the results of nessie test into UART, USB CDC or whatever function you want. You only have to initialize the function pointer to print the output where you want.

Sample code to print the output through USB CDC:

```
#include "aes256_nessie_test.h"

void main(void)
{
    /*
		INITIALIZATION OF USB CDC
    */

    // Redirect nessieOutput to usb_serial_putchar
    nessieOutput = &usb_serial_putchar;

    // Test all sets of nessie vectors
    nessieTest(1);
    nessieTest(2);
    nessieTest(3);
    nessieTest(4);
    nessieTest(5);
    nessieTest(6);
    nessieTest(7);
    nessieTest(8);
}
```

Nessie test vectors and output are located in <a href="https://www.cosic.esat.kuleuven.be/nessie/testvectors">https://www.cosic.esat.kuleuven.be/nessie/testvectors</a> Block Cipher -> Rijndael -> key size 256.

The output of all nessieTest functions are formatted in the same way as the file <b>aes256_nessie_test.txt</b>, so... you must save the output (using cutecom or similar hyperterminal program) into a file and check the differences between your file and <b>aes256_nessie_test.txt</b> using a diff viewer.

3- CTR block encryption
-----------------------
The passwords stored on the mooltipass will be encrypted using CTR block encryption, more information in: <a href="http://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#Counter_.28CTR.29"> Counter CTR </a>. We must decide how to generate the initialization vector. Here's an example of use of CTR encryption and decryption.

```
static uint8_t key[32] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b,
0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b,
0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };

static uint8_t iv[16] = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };


char text[32] = "this is my pass to encrypt";

void main(void)
{
	/*
		Stuff here
	*/

    // Declare aes256 context variable
    aes256CtrCtx_t ctx;

    // Save key and initialization vector inside context
    aes256CtrInit(&ctx, key, iv, 16);

	aes256CtrEncrypt(&ctx, text, sizeof(text));
	// here array pass has been encrypted inside text

    /*
        Decrypt
    */

	aes256CtrDecrypt(&ctx, text, sizeof(text));
	// decrypting make text to be "this is my pass to encrypt" again.
}
```
Data input in aes256CtrEncrypt and aes256CtrDecrypt must be multiple of 16 bytes length.


As said, we like to test our code and verify it has no bugs so... you can test CTR encryption implementation by using <b>aes256_ctr_test.c</b> functions and doing a diff against <b>aes256_ctr_test.txt</b>. CTR vectors used in aes256CtrTest are from <a href="http://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf"> http://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf</a>.
```
void main(void)
{

	/*
        INITIALIZATION OF USB CDC
    */

	// Redirect ctrTestOutput to usb_serial_putchar function
    ctrTestOutput = &usb_serial_putchar;

	aes256CtrTest();
}
```

4- Description of files
-----------------------
- AVR-cryptolib files used in this project:

```
aes.c -> AES256 implementation from http://www.literatecode.com/aes256 (with some things changed)
```

- Custom files done by mooltipass team:

```
aes256_nessie_test.c 	(only used for test)
aes256_ctr_test.c 		(only used for test)
aes256_ctr.c
```

- Files to check aes256_nessie and aes256_ctr tests:

```
aes256_nessie_test.txt
aes256_ctr_test.txt
```

5- Speed performance
--------------------

```
with #define BACK_TO_TABLES enabled
text     data     bss     dec    hex filename
1872       0       0    1872     750 aes.o

Time(1000 encryptions): 59433 ms


with #define BACK_TO_TABLES disabled
text    data     bss     dec     hex filename
2320       0       0    2320     910 aes.o

Time(1000 encryptions): 1204 ms
```
