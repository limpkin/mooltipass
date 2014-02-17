// Generate_Random_Bytes - This sketch makes use of the Entropy library
// to produce a serial of random 8 bit integers (bytes) that are streamed
// to the serial port of the arduino
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
  Serial.begin(115200);

  // This routine sets up the watch dog timer with interrupt handler to maintain a
  // pool of real entropy for use in sketches.  This mechanism is relatively slow
  // since it will only produce a little less than two 32-bit random values per 
  // second.
  Entropy.Initialize();

}

void loop()
{
  // When the random method is called with a single integer parameter it will return
  // a random integer that is in the range: 0 <= random_value < integer parameter
  Serial.println(Entropy.random(WDT_RETURN_BYTE));
}

