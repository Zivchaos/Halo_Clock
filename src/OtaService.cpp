#include "OtaService.h"

#include <ArduinoOTA.h>
#include <MD5Builder.h>
#include <Preferences.h>
#include <WiFi.h>

#include "Config.h"
#include "Ledring.h"
#include "Oled.h"
#include "OtaAuth.h"
#include "Version.h"

namespace
{
    bool initialized = false;
    bool updating = false;
    bool noticeVisible = false;
    uint32_t noticeStartedAt = 0;
    uint32_t noticeDurationMs = 0;
    uint8_t currentProgress = 0;
    bool sessionEnabled = false;
    Preferences preferences;
    char storedPasswordHash[33] = "";

    const char* configuredPasswordHash()
    {
        return storedPasswordHash[0] != '\0' ? storedPasswordHash : OtaAuth::passwordHash;
    }

    void startNotice(uint32_t durationMs)
    {
        noticeStartedAt = millis();
        noticeDurationMs = durationMs;
        noticeVisible = true;
    }

    const char* errorReason(ota_error_t error)
    {
        switch (error)
        {
            case OTA_AUTH_ERROR: return "AUTH";
            case OTA_BEGIN_ERROR: return "BEGIN";
            case OTA_CONNECT_ERROR: return "CONNECT";
            case OTA_RECEIVE_ERROR: return "RECEIVE";
            case OTA_END_ERROR: return "END";
            default: return "UNKNOWN";
        }
    }

    void showProgress(uint8_t percent)
    {
        char percentText[8];
        snprintf(percentText, sizeof(percentText), "%u%%", percent);
        Oled::status("UPDATING", percentText);
        LedRing::showOtaProgress(percent);
    }

    void configureOta()
    {
        if (configuredPasswordHash()[0] == '\0')
        {
            Serial.println("OTA DISABLED: password hash not configured");
            return;
        }
        ArduinoOTA.setHostname(Product::HOSTNAME);
        ArduinoOTA.setPasswordHash(configuredPasswordHash());

        ArduinoOTA.onStart([]() {
            updating = true;
            noticeVisible = false;
            currentProgress = 0;
            showProgress(currentProgress);
            Serial.println("OTA START");
        });

        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            const uint8_t percent = total == 0
                ? 0
                : static_cast<uint8_t>((static_cast<uint64_t>(progress) * 100U) / total);

            if (percent != currentProgress)
            {
                currentProgress = percent;
                showProgress(currentProgress);
                Serial.printf("OTA PROGRESS: %u%%\r\n", currentProgress);
            }
        });

        ArduinoOTA.onEnd([]() {
            updating = false;
            currentProgress = 100;
            LedRing::showOtaProgress(currentProgress);
            Oled::status("UPDATE COMPLETE", "RESTARTING");
            startNotice(Config::OTA_RESULT_NOTICE_MS);
            Serial.println("OTA COMPLETE");
        });

        ArduinoOTA.onError([](ota_error_t error) {
            updating = false;
            const char* reason = errorReason(error);
            Oled::status("UPDATE FAILED", reason);
            startNotice(Config::OTA_RESULT_NOTICE_MS);
            Serial.printf("OTA ERROR %u: %s\r\n", static_cast<unsigned int>(error), reason);
        });

        ArduinoOTA.begin();
        initialized = true;
        Oled::status("OTA READY", Product::HOSTNAME);
        startNotice(Config::OTA_READY_NOTICE_MS);
        Serial.printf("OTA READY: %s.local\r\n", Product::HOSTNAME);
    }
}

void OtaService::begin()
{
    initialized = false;
    updating = false;
    noticeVisible = false;
    currentProgress = 0;
    sessionEnabled = false;
    storedPasswordHash[0] = '\0';
    if (preferences.begin(Config::SETTINGS_NAMESPACE, false))
    {
        const String saved = preferences.getString("otaHash", "");
        if (saved.length() == 32) snprintf(storedPasswordHash, sizeof(storedPasswordHash), "%s", saved.c_str());
    }
}

void OtaService::update()
{
    if (!sessionEnabled || configuredPasswordHash()[0] == '\0')
    {
        return;
    }
    if (!initialized && WiFi.status() == WL_CONNECTED)
    {
        configureOta();
    }

    if (initialized && WiFi.status() == WL_CONNECTED)
    {
        ArduinoOTA.handle();
    }

    if (!updating && noticeVisible && millis() - noticeStartedAt >= noticeDurationMs)
    {
        noticeVisible = false;
    }
}

bool OtaService::isUpdating()
{
    return updating;
}

bool OtaService::isReady()
{
    return sessionEnabled && initialized && WiFi.status() == WL_CONNECTED;
}

bool OtaService::isDisplayReserved()
{
    return updating || noticeVisible;
}

uint8_t OtaService::progressPercent()
{
    return currentProgress;
}

bool OtaService::isConfigured()
{
    return configuredPasswordHash()[0] != '\0';
}

bool OtaService::enableForSession(const char* password)
{
    if (password == nullptr || password[0] == '\0') return false;
    MD5Builder digest;
    digest.begin();
    digest.add(password);
    digest.calculate();
    const String candidate = digest.toString();
    if (!isConfigured())
    {
        if (!preferences.putString("otaHash", candidate)) return false;
        snprintf(storedPasswordHash, sizeof(storedPasswordHash), "%s", candidate.c_str());
    }
    else if (candidate != configuredPasswordHash()) return false;
    sessionEnabled = true;
    return true;
}

bool OtaService::disableForSession()
{
    if (updating) return false;
    if (initialized) ArduinoOTA.end();
    initialized = false;
    sessionEnabled = false;
    return true;
}
