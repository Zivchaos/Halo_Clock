#include <Arduino.h>

#include "Halo.h"

void setup()
{
    Halo::begin();
}

void loop()
{
    Halo::update();
}