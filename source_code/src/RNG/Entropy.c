// Entropy - A entropy (random number) generator for the Arduino
//
// Copyright 2012 by Walter Anderson
//
// This file is part of Entropy, an Arduino library.
// Entropy is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Entropy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Entropy.  If not, see <http://www.gnu.org/licenses/>.
// 
// Edit:
// April 2014 - Changed Entropy.cpp to Entropy.c, cpp to c functions

#include <Entropy.h>
#include <util/atomic.h>

#define gWDT_buffer_SIZE 32
#define WDT_POOL_SIZE 2
uint8_t gWDT_buffer[gWDT_buffer_SIZE];
uint8_t gWDT_buffer_position;
uint8_t gWDT_loop_counter;
volatile uint8_t gWDT_pool_start;
volatile uint8_t gWDT_pool_end;
volatile uint8_t gWDT_pool_count;
volatile uint32_t gWDT_entropy_pool[WDT_POOL_SIZE];

union ENTROPY_LONG_WORD share_entropy;
static uint8_t byte_position=0;

//internal prototype functions
uint8_t EntropyAvailable(void);
uint32_t EntropyRandom(void);

/*! \fn void EntropyInit(void); 
 *  \brief This function initializes the global variables needed to implement 
 *  circular entropy pool and the buffer that holds the raw Timer 0 values 
 *  that are used to create the entropy pool. It initializes Timer0 and after
 *  that, it Initializes the Watch Dog Timer (WDT) to perform an interrupt 
 *  every 2048 clock cycles, (about 16 ms) which is as fast as it can be set.
*/
void EntropyInit(void)
{
    gWDT_buffer_position=0;
    gWDT_pool_start = 0;
    gWDT_pool_end = 0;
    gWDT_pool_count = 0;

    // configure TIMER0 here
    // Normal mode, internal clock with no preescaler
    TCCR0A = 0x00;
    TCCR0B = 0x01;    

    // Temporarily turn off interrupts, until WDT configured
    cli();
    // Use the MCU status register to reset flags for WDR, BOR, EXTR, and POWR                     
    MCUSR = 0;
    
    // WDT control register, This sets the Watchdog Change Enable (WDCE) flag, 
    // which is  needed to set the Watchdog system reset (WDE) enable and the 
    // Watchdog interrupt enable (WDIE)
    _WD_CONTROL_REG |= (1<<_WD_CHANGE_BIT) | (1<<WDE);
    _WD_CONTROL_REG = _BV(WDIE);

    // Turn interupts on
    sei();
}

/*! \fn uint32_t EntropyRandom(void)
 *  \brief This function returns a uniformly distributed random integer 
 *  in the range of [0,0xFFFFFFFF] as long as some entropy exists in the 
 *  pool and a 0 otherwise.  To ensure a proper random return the available() 
 *  function should be called first to ensure that entropy exists.
 *
 *  The pool is implemented as an 8 value circular buffer
 *  \return An unsigned 32bit random value from the entropy pool
*/
uint32_t EntropyRandom(void)
{
    uint8_t waiting;
    uint32_t retVal;
    while (gWDT_pool_count < 1)
    waiting += 1;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        retVal = gWDT_entropy_pool[gWDT_pool_start];
        gWDT_pool_start = (gWDT_pool_start + 1) % WDT_POOL_SIZE;
        --gWDT_pool_count;
    }
    return(retVal);
}

/*! \fn uint8_t EntropyRandom8(void)
 *  
 *  \brief This function returns one byte of a single 32-bit entropy value,
 *  while preserving the remaining bytes to be returned upon successive
 *  calls to the method.  This makes best use of the available entropy 
 *  pool when only bytes size chunks of entropy are needed.
 *  
 *  \return An unsigned 8bit random value
*/
uint8_t EntropyRandom8(void)
{
    uint8_t retVal8;

    // Read new values from the entropy pool only if all bytes have been
    // sent out
    if (byte_position == 0)
    {
        share_entropy.int32 = EntropyRandom();
    }
    retVal8 = share_entropy.int8[byte_position++];
    byte_position = byte_position % 4; 
    return(retVal8);
}

/*! \fn uint8_t EntropyBytesAvailable(void)
 *
 *  \brief This function returns the maximum available bytes from 
 *  the entropy pool and share_entropy variables.
 *
 *  \return the number of random bytes available
*/
uint8_t EntropyBytesAvailable(void)
{
    uint8_t nbytes = 0;

    // Check if we have data inside share_entropy variable
    if(byte_position == 0)
    {
        // EntropyAvailable * 4
        nbytes = EntropyAvailable() << 2;
    }
    else
    {
        // if we have 3 bytes used from share_entropy variable
        // we have 4 - 3 = 1byte available.
        nbytes = (EntropyAvailable() << 2) + (4-byte_position);
    }

    return nbytes;
}

/*! \fn uint8_t EntropyAvailable(void)
 *  \brief This function returns a unsigned char (8-bit) with the number
 *  of unsigned long values in the entropy pool
 *  \return The number of 32bit values inside the pool (for now 2 is the
 *  maximum value)
*/
uint8_t EntropyAvailable(void)
{
  return(gWDT_pool_count);
}

/*! \fn ISR(WDT_vect)
 *  \brief This interrupt service routine is called every time the WDT
 *  interrupt is triggered. With the default configuration that is 
 *  approximately once every 16ms, producing approximately two 32-bit 
 *  integer values every second. 
 *
 * The pool is implemented as an 8 value circular buffer.
*/

ISR(WDT_vect)
{
    // Record the Timer 0
    gWDT_buffer[gWDT_buffer_position] = TCNT0; 

    // every time the WDT interrupt is triggered
    gWDT_buffer_position++;
    if (gWDT_buffer_position >= gWDT_buffer_SIZE)
    {
        gWDT_pool_end = (gWDT_pool_start + gWDT_pool_count) % WDT_POOL_SIZE;
        // The following code is an implementation of Jenkin's one at a time hash
        // This hash function has had preliminary testing to verify that it
        // produces reasonably uniform random results when using WDT jitter
        // on a variety of Arduino/AVR platforms
        for(gWDT_loop_counter = 0; gWDT_loop_counter < gWDT_buffer_SIZE; ++gWDT_loop_counter)
        {
            gWDT_entropy_pool[gWDT_pool_end] += gWDT_buffer[gWDT_loop_counter];
            gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 10);
            gWDT_entropy_pool[gWDT_pool_end] ^= (gWDT_entropy_pool[gWDT_pool_end] >> 6);
        }

        gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 3);
        gWDT_entropy_pool[gWDT_pool_end] ^= (gWDT_entropy_pool[gWDT_pool_end] >> 11);
        gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 15);
        gWDT_entropy_pool[gWDT_pool_end] = gWDT_entropy_pool[gWDT_pool_end];
        gWDT_buffer_position = 0; // Start collecting the next 32 bytes of Timer 0 counts

        // The entropy pool is full
        if (gWDT_pool_count == WDT_POOL_SIZE)
        {
            gWDT_pool_start = (gWDT_pool_start + 1) % WDT_POOL_SIZE;
        }
        // Add another unsigned long (32 bits) to the entropy pool
        else
        {
          ++gWDT_pool_count;
        }
    }
}
