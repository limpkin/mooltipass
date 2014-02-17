// Generate_Random_Numbers - This sketch makes use of the Entropy library
// to produce a sequence of random integers and floating point values.
// to demonstrate the use of the entropy library
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

void setup()
{
  uint8_t random_byte;
  uint16_t random_int;
  uint32_t random_long;
  float random_float;
  
  Serial.begin(115200);

  // This routine sets up the watch dog timer with interrupt handler to maintain a
  // pool of real entropy for use in sketches.  This mechanism is relatively slow
  // since it will only produce a little less than two 32-bit random values per 
  // second.
  Entropy.Initialize();

  // Simulate a coin flip
  random_byte = Entropy.random(2); // return a 0 or a 1
  Serial.print("The coin was a ");
  if (random_byte == 0)
    Serial.println("tail.");
  else
    Serial.println("heads.");

  // Simulate rolling a six sided die; i.e. produce the numbers 1 through 6 with 
  // equal probability
  random_byte = Entropy.random(1,7); // returns a value from 1 to 6
  Serial.print("The die rolled a ");
  Serial.println(random_byte);
  
  // Return a random integer (0 - 65365)
  random_int = Entropy.random(WDT_RETURN_WORD);
  Serial.print("The random integer was ");
  Serial.println(random_int);
  
  // Generate a random floating point value in the range of 0-1
  random_long = Entropy.random() & 0xFFFFFF; // The arduino only has 24 bits mantissa
  random_float = random_long / 16777216.0;   // Maximum 24 bit integer value + 1.0
  Serial.print("The random floating point was ");
  Serial.println(random_float);
}

void loop()
{
}

