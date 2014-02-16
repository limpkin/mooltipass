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

#ifndef Die_h
#define Die_h

class Die
{
public:
  void Initialize(int a, int b, int c, int d, int e, int f, int g);
  void Show(unsigned char value);
 private:
  int led_a, led_b, led_c, led_d, led_e, led_f, led_g;
  void On(void);
  void Off(void);
  void Error(void);
};
#endif
