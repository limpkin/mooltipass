// Dice - Application to simulate a six sided die, using the Entropy
//        true random number library

// The die is 7 leds wired as shown below
// A   B  A=D0, B=D1
// C D E  C=D2, D=D3, E=D4
// F   G  F=D5, G=D6

#include <stdint.h>
#include <Entropy.h>
#include "Die.h"

const int RollButton=14;
const int Die1=9;
const int Die2=10;
Die Dice[2];
byte roll[2];

void setup()
{
  Entropy.Initialize();
  Dice[0].Initialize(7,8,2,3,4,5,6);
  Dice[1].Initialize(7,8,2,3,4,5,6);
  pinMode(RollButton,INPUT_PULLUP);
  pinMode(Die1, OUTPUT);
  pinMode(Die2, OUTPUT);
  digitalWrite(Die1, LOW);
  digitalWrite(Die2, LOW);
  roll[0] = Entropy.random(1,7);
  roll[1] = Entropy.random(1,7);
}

void loop()
{
  if (digitalRead(RollButton) == LOW)
  {
    while (digitalRead(RollButton) == LOW);
    roll[0] = Entropy.random(1,7);
    roll[1] = Entropy.random(1,7);
  }
  digitalWrite(Die1, HIGH);
  Dice[0].Show(roll[0]);
  delay(100);
  PORTD = 0x00;
  PORTB = 0x00;
  digitalWrite(Die2, HIGH);
  Dice[0].Show(roll[1]);
  delay(100);
  PORTD = 0x00;
  PORTB = 0x00;
}

