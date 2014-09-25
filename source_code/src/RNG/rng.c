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
/*! \file   rng.c
*   \brief  Random Number Generation
* 
*   Created: 19/09/2014 18:30
*   Author: Miguel A. Borrego
*/

// We will use ATOMIC operations
#include <util/atomic.h>
#include <rng.h>

// Number of values to be passed to Jenkins hash function
#define TIMER_BUFFER_SIZE               32

// Number of Random bytes to be saved in a Buffer 
// Better if Buffer size is power of 2
#define RNG_BUFFER_SIZE                 32

uint8_t timer_buffer[TIMER_BUFFER_SIZE];
volatile uint8_t timer_buffer_index;

volatile uint8_t rng_buffer_index;
volatile uint8_t rng_buffer_last_valid_value;
volatile uint8_t rng_buffer_count;
volatile uint8_t rng_buffer_count_bytes;
volatile uint32_t rng_buffer[RNG_BUFFER_SIZE];

//internal prototype functions
static uint32_t jenkins_one_at_a_time_hash(uint8_t *key, uint8_t len);
static uint32_t rngGet32(void);
static uint8_t rngGet8(void);

typedef union
{
    uint32_t fourByteAccess;
    uint8_t  oneByteAccess[4];
} rng_item_t;

typedef struct
{
    rng_item_t number;
    uint8_t    pendingBytes;
} rng_struct_t;

rng_struct_t rngValue;


/*! \fn ISR(WDT_vect)
 *  \brief This interrupt service routine is called every time the WDT
 *  interrupt is triggered, 16ms.
*/
ISR(WDT_vect)
{
    // Save timer0 value
    // every time the WDT interrupt is triggered
    timer_buffer[timer_buffer_index] = TCNT0;
    timer_buffer_index++;

    if (timer_buffer_index >= TIMER_BUFFER_SIZE)
    {
        // Start collecting 32 bytes more of TCNT0
        timer_buffer_index = 0;

        // The following code is an implementation of Jenkin's one at a time hash
        // it produces reasonably uniform random results when using WDT jitter

        rng_buffer[rng_buffer_index] = jenkins_one_at_a_time_hash(timer_buffer, TIMER_BUFFER_SIZE);
        // circular buffer, index increment
        rng_buffer_index = (rng_buffer_index+1) % RNG_BUFFER_SIZE;

        // If random buffer is full, increment last valid value
        if (rng_buffer_count == RNG_BUFFER_SIZE)
        {
            rng_buffer_last_valid_value = rng_buffer_index;
        }
        // we have added a new value and random buffer is not full,
        // so, increment buffer elements count
        else
        {
          ++rng_buffer_count;
        }
    }
}

/*! \fn void rngInit(void); 
 *  \brief This function initializes the global variables needed to implement 
 *  circular rng buffer and timer buffer.
*/
void rngInit(void)
{    
    // Initialize vars
    timer_buffer_index = 0;
    rng_buffer_index = 0;
    rng_buffer_last_valid_value = 0;
    rng_buffer_count = 0;
    rngValue.number.fourByteAccess = 0;
    rngValue.pendingBytes = 0;

    // Configure TIMER0 here
    // Normal mode, internal clock with no prescaler
    TCCR0A = 0x00;
    TCCR0B = 0x01;    

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {    
        // Use the MCU status register to reset flags for WDR, BOR, EXTR, and POWR                    
        MCUSR = 0;
    
        // watchdog interrupt enable (WDIE)
        _WD_CONTROL_REG |= (1<<_WD_CHANGE_BIT) | (1<<WDE);
        _WD_CONTROL_REG = _BV(WDIE);
    }    
}

/*! \fn static uint32_t rngGet32(void)
 *  \brief This function returns 4 bytes from the rng buffer
 *  \return 4 byte random values
*/
static uint32_t rngGet32(void)
{
    uint32_t retVal;
    
    /* Take care, this is a blocking function */
    while ( rng_buffer_count < 1);
    
    /* critical region here */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        retVal = rng_buffer[rng_buffer_last_valid_value];
        rng_buffer_last_valid_value = (rng_buffer_last_valid_value + 1) % RNG_BUFFER_SIZE;
        --rng_buffer_count;
    }

    rngValue.pendingBytes = 0;
    
    return(retVal);
}

/*! \fn static uint8_t rngGet8(void)
 *  \brief This function returns 1 byte from a single 32-bit random value
 *  \return 1 byte random value
*/
static uint8_t rngGet8(void)
{
    // If no more bytes available, read more bytes
    if (rngValue.pendingBytes == 0)
    {
        rngValue.number.fourByteAccess = rngGet32();
    rngValue.pendingBytes = 4;
    }
    return  rngValue.number.oneByteAccess[--rngValue.pendingBytes];	
}

/*! \fn fillArrayWithRandomBytes(uint8_t* buffer, uint8_t nb_bytes)
 *  \brief  Fill array with random bytes
 *  \param  buffer      The array
 *  \param  nb_bytes    The number of bytes
*/
void fillArrayWithRandomBytes(uint8_t* buffer, uint8_t nb_bytes)
{
    for (uint8_t i = 0; i < nb_bytes; i++)
    {
        buffer[i] = rngGet8();
    }
}

/*! \fn static uint32_t jenkins_one_at_a_time_hash(uint8_t* key, uint8_t len)
 *  \brief  Generates a unique 4 byte hash from a buffer
 *  \param  key  the input buffer
 *  \param  len  length of the buffer
*/
static uint32_t jenkins_one_at_a_time_hash(uint8_t* key, uint8_t len)
{
    uint8_t i;
    uint32_t hash;
    hash = 0;
    for(i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}
