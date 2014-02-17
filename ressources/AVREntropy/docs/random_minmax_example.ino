#include <Entropy.h>

void setup()
{
  uint8_t random_byte;

  Entropy.Initialize();

  // Simulate rolling a six sided die; i.e. produce the numbers 1 through 6 with 
  // equal probability
  random_byte = Entropy.random(1,7); // returns a value from 1 to 6
}

void loop()
{
}