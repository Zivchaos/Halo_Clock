#pragma once

#include <stddef.h>

enum class RedAlertParseResult
{
    OK,
    MALFORMED_RESPONSE,
    INVALID_RESPONSE
};

struct RedAlertReading
{
    bool sourceAlert = false;
    bool matchesSelection = false;
    char id[48] = "";
    char title[48] = "";
    char areas[192] = "";
};

class RedAlertProvider
{
public:
    // Accepts the Home Front Command object format and community notification
    // arrays. Every active notification is considered, not only the first.
    static RedAlertParseResult parseResponse(
        const char* payload,
        size_t length,
        const char* selectedLocations,
        RedAlertReading& reading);
};
