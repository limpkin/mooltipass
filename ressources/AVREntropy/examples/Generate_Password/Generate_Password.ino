// Entropy_Password - This sketch demonstrates using the Entropy library to generate secure random passowrds where
//                    there are 64 possible values for each character and that the password contains 8 characters
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

// This routine will convert a 6 bit value (0-63) to an acceptible password character
char mapChar(uint32_t parm)
{
  char retval;
  if (parm < 10)           // map 0..9 to ascii 0..9
    retval = char(48 + parm);
  else if (parm < 11)      // map 10 to -
    retval = '-';
  else if (parm < 12)      // map 11 to +
    retval = '.';
  else if (parm < 38)      // map 12 to 37 to ascii A..Z
    retval = char(53 + parm);
  else if (parm < 64)      // map 38 to 63 to ascii a..z
    retval = char(59 + parm);
  else
    retval = 0;            // if parm is invalid return null  
  return(retval);
}

// This routine uses the Entropy library to obtain truly random 6 bit values
// and to map that to an eight character cryptographically secure password
char *getPassword(char *pw)
{
  char ch;
  int indx;
  uint32_t tmp;
  
  
  for (indx=0; indx<8; indx++)
  { 
    tmp = mapChar(Entropy.random(64));
    pw[indx] = (char) tmp;
  }
  pw[8] = 0;
  return(pw);
}

void setup()
{
  char pw[9];
  
  Serial.begin(115200);
  Entropy.Initialize();

  Serial.print("Password: ");
  Serial.println(getPassword(pw));

  // Here are some examples of passwords generated with this sketch on various arduino hardware:
  //
  // Password: z6wvnEpC
  // Password: 9VCTpwfj
  // Password: ASXHjEQq
  // Password: 1h-.Chh1
  // Password: vJ7aC.kh
  // Password: lYZB45Dh
  // Password: JrNdeF68
  // Password: Y0NE0-uM
  // Password: Y3F1cQDC
  // Password: iPPp0dOX
  // Password: 3EhfP7zU
  // Password: cYurD0nQ
}

void loop()
{ 
}

