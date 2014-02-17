#include <Entropy.h>

void setup()
{
  uint8_t random_byte;
  uint16_t random_int;

  Entropy.Initialize();

  // Simulate a coin flip
  random_byte = Entropy.random(2); // return a 0 or a 1

  // Return a random integer (0 - 65365)
  random_int = Entropy.random(WDT_RETURN_WORD);
}

void loop()
{
}