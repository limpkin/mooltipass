// Use Entropy library functionality to seed the avr-libc pseudo-
//   random number generator
//
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

void setup()
{
  uint32_t seed_value;

  Serial.begin(115200);

  // This routine sets up the watch dog timer with interrupt handler to maintain a
  // pool of real entropy for use in sketches.  This mechanism is relatively slow
  // since it will only produce a little less than two 32-bit random values per 
  // second.
  Entropy.Initialize();

  // The random method returns an unsigned 32-bit value, which can be cast as a 
  // signed value if needed.  The function will wait until sufficient entropy is
  // available to return, which could cause delays of up to approximately 500ms
  seed_value = Entropy.random();
  
  Serial.print("Seed value = ");
  Serial.println(seed_value);
  
  // By using the Entropy library to seed the normal pseudo-random number generator which 
  // ensures that the standard libraries random number generator will provide different starting
  // values each time the sketch is run.  This performs much better than the normal
  // randomSeed(analogRead(0)).
  randomSeed(seed_value);
  
  // Here are some typical values produced when this sketch was run 25 times
  // Seed value = 2054931336
  // Seed value = 2689566361
  // Seed value = 953766268
  // Seed value = 1799328995
  // Seed value = 3050792285
  // Seed value = 607814576
  // Seed value = 2314140972
  // Seed value = 1573721872
  // Seed value = 507815617
  // Seed value = 1678088733
  // Seed value = 2655736882
  // Seed value = 2307754733
  // Seed value = 2704765785
  // Seed value = 2924991904
  // Seed value = 823689487
  // Seed value = 3858651144
  // Seed value = 183355835
  // Seed value = 1792358414
  // Seed value = 4175350052
  // Seed value = 467439649
  // Seed value = 1884043255
  // Seed value = 3786687950
  // Seed value = 1349957766
  // Seed value = 652610269
  //
  // The more normal randomSeed(analogRead(0)) produces far less random seed
  // values as showm in the following 25 examples:
  // Seed value = 303
  // Seed value = 326
  // Seed value = 327
  // Seed value = 326
  // Seed value = 326
  // Seed value = 328
  // Seed value = 328
  // Seed value = 328
  // Seed value = 330
  // Seed value = 328
  // Seed value = 328
  // Seed value = 329
  // Seed value = 327
  // Seed value = 328
  // Seed value = 328
  // Seed value = 329
  // Seed value = 329
  // Seed value = 329
  // Seed value = 331
  // Seed value = 331
  // Seed value = 330
  // Seed value = 331
  // Seed value = 329
  // Seed value = 329
  // Seed value = 329
}

void loop()
{
}

