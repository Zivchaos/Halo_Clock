#pragma once

#include "AppTypes.h"

struct HaloTheme
{
    const char* name;
    RgbColor tick;
    RgbColor minute;
    RgbColor hour;
    RgbColor hourSide;
    RgbColor second;
    RgbColor accent;
};

class Themes
{
public:
    static const HaloTheme& get(ThemeId id);
};
