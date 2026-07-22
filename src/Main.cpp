#include <Arduino.h>
#include <esp_err.h>
#include <esp_task_wdt.h>

#include "Config.h"
#include "Halo.h"

namespace
{
    void startWatchdog()
    {
        const esp_err_t initialized = esp_task_wdt_init(Config::WATCHDOG_TIMEOUT_SECONDS, true);
        if (initialized != ESP_OK && initialized != ESP_ERR_INVALID_STATE)
        {
            Serial.printf("WATCHDOG ERROR: INIT %d\r\n", static_cast<int>(initialized));
            return;
        }

        const esp_err_t subscribed = esp_task_wdt_add(nullptr);
        if (subscribed != ESP_OK && subscribed != ESP_ERR_INVALID_ARG)
        {
            Serial.printf("WATCHDOG ERROR: SUBSCRIBE %d\r\n", static_cast<int>(subscribed));
            return;
        }

        Serial.printf("WATCHDOG: ENABLED (%lu SECONDS)\r\n", static_cast<unsigned long>(Config::WATCHDOG_TIMEOUT_SECONDS));
    }
}

void setup()
{
    Halo::begin();
    startWatchdog();
}

void loop()
{
    Halo::update();
    esp_task_wdt_reset();
}
