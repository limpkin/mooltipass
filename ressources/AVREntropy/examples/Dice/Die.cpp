// Die - A class to handle the display of a six sided die, using
//       seven light emitting diodes
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

#include <Arduino.h>
#include "Die.h"

// Initializes the class with pin numbers to use as outputs for the
// seven light emitting diodes use to display a die.  The diodes are
// arranged as:
// a   b
// c d e
// f   g
void Die::Initialize(int a, int b, int c, int d, int e, int f, int g)
{
  // Store pin assignments
  led_a = a;
  led_b = b;
  led_c = c;
  led_d = d;
  led_e = e;
  led_f = f;
  led_g = g;

  // Configure pin modes
  pinMode(led_a, OUTPUT);
  digitalWrite(led_a, LOW);
  pinMode(led_b, OUTPUT);
  digitalWrite(led_b, LOW);
  pinMode(led_c, OUTPUT);
  digitalWrite(led_c, LOW);
  pinMode(led_d, OUTPUT);
  digitalWrite(led_d, LOW);
  pinMode(led_e, OUTPUT);
  digitalWrite(led_e, LOW);
  pinMode(led_f, OUTPUT);
  digitalWrite(led_f, LOW);
  pinMode(led_g, OUTPUT);
  digitalWrite(led_g, LOW);
}

// Turn on the appropriate LED's based upon value
void Die::Show(unsigned char value)
{
  Off();
  delay(200);
  switch (value)
  {
    case 1:
      digitalWrite(led_d, HIGH);      
      break;
    case 2:
      digitalWrite(led_b, HIGH);
      digitalWrite(led_f, HIGH);
      break;
    case 3:
      digitalWrite(led_b, HIGH);
      digitalWrite(led_d, HIGH);
      digitalWrite(led_f, HIGH);
      break;
    case 4:
      digitalWrite(led_a, HIGH);
      digitalWrite(led_b, HIGH);
      digitalWrite(led_f, HIGH);
      digitalWrite(led_g, HIGH);
      break;
    case 5:
      digitalWrite(led_a, HIGH);
      digitalWrite(led_b, HIGH);
      digitalWrite(led_d, HIGH);
      digitalWrite(led_f, HIGH);
      digitalWrite(led_g, HIGH);
      break;
    case 6:
      digitalWrite(led_a, HIGH);
      digitalWrite(led_b, HIGH);
      digitalWrite(led_c, HIGH);
      digitalWrite(led_e, HIGH);
      digitalWrite(led_f, HIGH);
      digitalWrite(led_g, HIGH);
      break;
    default:
      Error();
  }
}

void Die::On(void)
{
  digitalWrite(led_a, HIGH);
  digitalWrite(led_b, HIGH);
  digitalWrite(led_c, HIGH);
  digitalWrite(led_d, HIGH);
  digitalWrite(led_e, HIGH);
  digitalWrite(led_f, HIGH);
  digitalWrite(led_g, HIGH);
}

void Die::Off(void)
{
  digitalWrite(led_a, LOW);
  digitalWrite(led_b, LOW);
  digitalWrite(led_c, LOW);
  digitalWrite(led_d, LOW);
  digitalWrite(led_e, LOW);
  digitalWrite(led_f, LOW);
  digitalWrite(led_g, LOW);
}

void Die::Error(void)
{
  for (int i=0; i<10; i++)
  {
    delay(50);
    On();
    delay(50);
    Off();
  }
}
