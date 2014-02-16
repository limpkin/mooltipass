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

#ifndef Entropy_h
#define Entropy_h

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

const uint32_t WDT_RETURN_BYTE=256;
const uint32_t WDT_RETURN_WORD=65536;

union ENTROPY_LONG_WORD 
{
  uint32_t int32;
  uint16_t int16[2];
  uint8_t int8[4];
};

class EntropyClass
{
public:
  void Initialize(void);
  uint32_t random(void);
  uint32_t random(uint32_t max);
  uint32_t random(uint32_t min, uint32_t max);
  uint8_t randomByte(void);
  uint16_t randomWord(void);
  uint8_t available(void);
 private:
  ENTROPY_LONG_WORD share_entropy;
  uint32_t retVal;
  uint8_t random8(void);
  uint16_t random16(void);
};
extern EntropyClass Entropy;
#endif
