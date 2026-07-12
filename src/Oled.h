#pragma once

class Oled
{
public:

    static void begin();

    static void splash();

    static void status(
        const char* line1,
        const char* line2);

};