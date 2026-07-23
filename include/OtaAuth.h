#pragma once

// Copy LocalSecrets.example.h to LocalSecrets.h and provide a password hash.
// LocalSecrets.h is intentionally ignored by Git.
#if __has_include("LocalSecrets.h")
#include "LocalSecrets.h"
#endif

#ifndef HALO_OTA_PASSWORD_HASH
#define HALO_OTA_PASSWORD_HASH ""
#endif

namespace OtaAuth
{
    constexpr const char* passwordHash = HALO_OTA_PASSWORD_HASH;
    constexpr bool configured = passwordHash[0] != '\0';
}
