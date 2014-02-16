#include <Entropy.h>

void setup()
{
  Entropy.Initialize();
}

void loop()
{
  if (Entropy.available())
     randomSeed(Entropy.random());
}