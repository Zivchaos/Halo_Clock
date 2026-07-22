#include "RedAlertProvider.h"

#include <ArduinoJson.h>
#include <cstring>

namespace
{
    bool isSelected(const char* selectedLocations, const char* area)
    {
        if (selectedLocations == nullptr || area == nullptr || area[0] == '\0') return false;
        const size_t areaLength = strlen(area);
        const char* line = selectedLocations;
        while (*line != '\0')
        {
            const char* lineEnd = strchr(line, '\n');
            const size_t lineLength = lineEnd == nullptr ? strlen(line) : static_cast<size_t>(lineEnd - line);
            if (lineLength == areaLength && strncmp(line, area, areaLength) == 0) return true;
            if (lineEnd == nullptr) break;
            line = lineEnd + 1;
        }
        return false;
    }

    bool alreadyListed(const char* list, const char* area)
    {
        if (list[0] == '\0') return false;
        const size_t areaLength = strlen(area);
        const char* item = list;
        while (*item != '\0')
        {
            const char* itemEnd = strstr(item, ", ");
            const size_t itemLength = itemEnd == nullptr ? strlen(item) : static_cast<size_t>(itemEnd - item);
            if (itemLength == areaLength && strncmp(item, area, areaLength) == 0) return true;
            if (itemEnd == nullptr) break;
            item = itemEnd + 2;
        }
        return false;
    }

    void appendArea(RedAlertReading& reading, const char* area)
    {
        if (alreadyListed(reading.areas, area)) return;
        if (reading.areas[0] != '\0') strlcat(reading.areas, ", ", sizeof(reading.areas));
        strlcat(reading.areas, area, sizeof(reading.areas));
    }

    void processNotification(JsonObject notification, const char* selectedLocations, RedAlertReading& reading)
    {
        JsonArray areas = notification["data"].as<JsonArray>();
        if (areas.isNull()) areas = notification["cities"].as<JsonArray>();
        const char* id = notification["id"] | "";
        if (id[0] == '\0') id = notification["notificationId"] | "";
        if (areas.isNull() || areas.size() == 0 || id[0] == '\0') return;

        reading.sourceAlert = true;
        if (reading.id[0] == '\0')
        {
            snprintf(reading.id, sizeof(reading.id), "%s", id);
            snprintf(reading.title, sizeof(reading.title), "%s", notification["title"] | "Community Red Alert");
        }
        for (JsonVariant value : areas)
        {
            const char* area = value.as<const char*>();
            if (isSelected(selectedLocations, area))
            {
                reading.matchesSelection = true;
                appendArea(reading, area);
            }
        }
    }
}

RedAlertParseResult RedAlertProvider::parseResponse(
    const char* payload,
    size_t length,
    const char* selectedLocations,
    RedAlertReading& reading)
{
    reading = RedAlertReading{};
    if (payload == nullptr) return RedAlertParseResult::MALFORMED_RESPONSE;
    JsonDocument document;
    if (deserializeJson(document, payload, length)) return RedAlertParseResult::MALFORMED_RESPONSE;

    if (document.is<JsonArray>())
    {
        for (JsonVariant value : document.as<JsonArray>()) processNotification(value.as<JsonObject>(), selectedLocations, reading);
        return RedAlertParseResult::OK;
    }
    if (document.is<JsonObject>())
    {
        processNotification(document.as<JsonObject>(), selectedLocations, reading);
        return RedAlertParseResult::OK;
    }
    return RedAlertParseResult::INVALID_RESPONSE;
}
