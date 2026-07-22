#pragma once

#include <Arduino.h>

struct RedAlertData
{
    bool enabled = false;
    bool active = false;
    bool stale = false;
    bool updating = false;
    uint32_t lastSuccessfulUpdateMs = 0;
    char id[48] = "";
    char title[48] = "";
    char areas[192] = "";
    char error[64] = "disabled";
};

class RedAlertService
{
public:
    static void begin();
    static void update();
    static RedAlertData snapshot();
    static bool settingsChanged();
    static bool simulate();
};
