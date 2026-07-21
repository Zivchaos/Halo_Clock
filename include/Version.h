#pragma once

#define HALO_CST_PRODUCT_NAME "HALO CST"
#define HALO_CST_EXPANDED_NAME "Connected Smart Timepiece"
#define HALO_CST_FIRMWARE_VERSION "1.0.0"
#define HALO_CST_HOSTNAME "halo-cst"
#define HALO_CST_SETUP_AP_NAME "HALO-CST-Setup"

namespace Product
{
    constexpr const char* NAME = HALO_CST_PRODUCT_NAME;
    constexpr const char* SHORT_NAME = "CST";
    constexpr const char* EXPANDED_NAME = HALO_CST_EXPANDED_NAME;
    constexpr const char* PROJECT_NAME = "HALO CST \xE2\x80\x94 " HALO_CST_EXPANDED_NAME;
    constexpr const char* FIRMWARE_VERSION = HALO_CST_FIRMWARE_VERSION;
    constexpr const char* HOSTNAME = HALO_CST_HOSTNAME;
    constexpr const char* SETUP_AP_NAME = HALO_CST_SETUP_AP_NAME;
    constexpr const char* PROJECT_URL = "https://github.com/Zivchaos/Halo_Clock";

    constexpr const char* BUILD_DATE = __DATE__;
    constexpr const char* BUILD_TIME = __TIME__;
}
