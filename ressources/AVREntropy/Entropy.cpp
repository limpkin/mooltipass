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

#include <Entropy.h>
#include <util/atomic.h>

const uint8_t gWDT_buffer_SIZE=32;
const uint8_t WDT_POOL_SIZE=8;
const uint8_t WDT_MAX_8INT=0xFF;
const uint16_t WDT_MAX_16INT=0xFFFF;
const uint32_t WDT_MAX_32INT=0xFFFFFFFF;
uint8_t gWDT_buffer[gWDT_buffer_SIZE];
uint8_t gWDT_buffer_position;
uint8_t gWDT_loop_counter;
volatile uint8_t gWDT_pool_start;
volatile uint8_t gWDT_pool_end;
volatile uint8_t gWDT_pool_count;
volatile uint32_t gWDT_entropy_pool[WDT_POOL_SIZE];

// This function initializes the global variables needed to implement the circular entropy pool and
// the buffer that holds the raw Timer 1 values that are used to create the entropy pool.  It then
// Initializes the Watch Dog Timer (WDT) to perform an interrupt every 2048 clock cycles, (about 
// 16 ms) which is as fast as it can be set.
void EntropyClass::Initialize(void)
{
  gWDT_buffer_position=0;
  gWDT_pool_start = 0;
  gWDT_pool_end = 0;
  gWDT_pool_count = 0;
  cli();                         // Temporarily turn off interrupts, until WDT configured
  MCUSR = 0;                     // Use the MCU status register to reset flags for WDR, BOR, EXTR, and POWR
  _WD_CONTROL_REG |= (1<<_WD_CHANGE_BIT) | (1<<WDE);
  // WDTCSR |= _BV(WDCE) | _BV(WDE);// WDT control register, This sets the Watchdog Change Enable (WDCE) flag, which is  needed to set the 
  _WD_CONTROL_REG = _BV(WDIE);            // Watchdog system reset (WDE) enable and the Watchdog interrupt enable (WDIE)
  sei();                         // Turn interupts on
}

// This function returns a uniformly distributed random integer in the range
// of [0,0xFFFFFFFF] as long as some entropy exists in the pool and a 0
// otherwise.  To ensure a proper random return the available() function
// should be called first to ensure that entropy exists.
//
// The pool is implemented as an 8 value circular buffer
uint32_t EntropyClass::random(void)
{
  uint8_t waiting;
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

// This function returns one byte of a single 32-bit entropy value, while preserving the remaining bytes to
// be returned upon successive calls to the method.  This makes best use of the available entropy pool when
// only bytes size chunks of entropy are needed.  Not available to public use since there is a method of using
// the default random method for the end-user to achieve the same results.  This internal method is for providing
// that capability to the random method, shown below
uint8_t EntropyClass::random8(void)
{
  static uint8_t byte_position=0;
  uint8_t retVal8;

  if (byte_position == 0)
    share_entropy.int32 = random();
  retVal8 = share_entropy.int8[byte_position++];
  byte_position = byte_position % 4;
  return(retVal8);
}

// This function returns one word of a single 32-bit entropy value, while preserving the remaining word to
// be returned upon successive calls to the method.  This makes best use of the available entropy pool when
// only word sized chunks of entropy are needed.  Not available to public use since there is a method of using
// the default random method for the end-user to achieve the same results.  This internal method is for providing
// that capability to the random method, shown below
uint16_t EntropyClass::random16(void)
{
  static uint8_t word_position=0;
  uint16_t retVal16;

  if (word_position == 0)
    share_entropy.int32 = random();
  retVal16 = share_entropy.int16[word_position++];
  word_position = word_position % 2;
  return(retVal16);
}

// This function returns a uniformly distributed integer in the range of 
// of [0,max).  The added complexity of this function is required to ensure
// a uniform distribution since the naive modulus max (% max) introduces
// bias for all values of max that are not powers of two.
//
// The loops below are needed, because there is a small and non-uniform chance
// That the division below will yield an answer = max, so we just get
// the next random value until answer < max.  Which prevents the introduction
// of bias caused by the division process.  This is why we can't use the 
// simpler modulus operation which introduces significant bias for divisors
// that aren't a power of two
uint32_t EntropyClass::random(uint32_t max)
{
  uint32_t slice;

  if (max < 2)
    retVal=0;
  else
    {
      retVal = WDT_MAX_32INT;
      if (max <= WDT_MAX_8INT) // If only byte values are needed, make best use of entropy
	{                      // by diving the long into four bytes and using individually
	  slice = WDT_MAX_8INT / max;
	  while (retVal >= max)
	    retVal = random8() / slice;
	} 
      else if (max <= WDT_MAX_16INT) // If only word values are need, make best use of entropy
	{                            // by diving the long into two words and using individually
	  slice = WDT_MAX_16INT / max;
	  while (retVal >= max)
	    retVal = random16() / slice;
	} 
      else 
	{
	  slice = WDT_MAX_32INT / max;
	  while (retVal >= max)           
	    retVal = random() / slice;
	}                                 
    }
  return(retVal);
}

// This function returns a uniformly distributed integer in the range of 
// of [min,max).  
uint32_t EntropyClass::random(uint32_t min, uint32_t max)
{
  uint32_t slice, tmp_random, tmax;

  tmax = max - min;
  if (tmax < 2)
    retVal=0;
  else
    {
      tmp_random = random(tmax);
      retVal = min + tmp_random;
    }
  return(retVal);
}

// This function returns a unsigned char (8-bit) with the number of unsigned long values
// in the entropy pool
uint8_t EntropyClass::available(void)
{
  return(gWDT_pool_count);
}

// This interrupt service routine is called every time the WDT interrupt is triggered.
// With the default configuration that is approximately once every 16ms, producing 
// approximately two 32-bit integer values every second. 
//
// The pool is implemented as an 8 value circular buffer
ISR(WDT_vect)
{
#if defined( __AVR_ATtiny25__ ) || defined( __AVR_ATtiny45__ ) || defined( __AVR_ATtiny85__ )  
  gWDT_buffer[gWDT_buffer_position] = TCNT0;
#else
  gWDT_buffer[gWDT_buffer_position] = TCNT1L; // Record the Timer 1 low byte (only one needed) 
#endif
  gWDT_buffer_position++;                     // every time the WDT interrupt is triggered
  if (gWDT_buffer_position >= gWDT_buffer_SIZE)
  {
    gWDT_pool_end = (gWDT_pool_start + gWDT_pool_count) % WDT_POOL_SIZE;
    // The following code is an implementation of Jenkin's one at a time hash
    // This hash function has had preliminary testing to verify that it
    // produces reasonably uniform random results when using WDT jitter
    // on a variety of Arduino platforms
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
    gWDT_buffer_position = 0; // Start collecting the next 32 bytes of Timer 1 counts
    if (gWDT_pool_count == WDT_POOL_SIZE) // The entropy pool is full
      gWDT_pool_start = (gWDT_pool_start + 1) % WDT_POOL_SIZE;  
    else // Add another unsigned long (32 bits) to the entropy pool
      ++gWDT_pool_count;
  }
}

// The library implements a single global instance.  There is no need, nor will the library 
// work properly if multiple instances are created.
EntropyClass Entropy;
