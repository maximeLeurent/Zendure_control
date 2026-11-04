#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>
#include <memory>
#include <new>
#include <mbedtls/base64.h>
#include <mbedtls/x509_crt.h>

inline const char *get_secret(const char *name, bool certificate = false)
{
    // Keep the registry alive so returned strings remain valid between calls.
    static JsonDocument secrets;
    static bool loaded = false;
    if (!loaded)
    {
        // Base64 decoding never requires more bytes than the encoded input.
        constexpr size_t encodedLength = sizeof(KEEPASS_SECRETS_BASE64) - 1;
        std::unique_ptr<unsigned char[]> decoded(new (std::nothrow) unsigned char[encodedLength + 1]);
        if (!decoded)
        {
            Serial.println("Unable to allocate memory for KeePass secrets.");
            return nullptr;
        }
        size_t decodedLength = 0;
        const int status = mbedtls_base64_decode(
            decoded.get(), encodedLength, &decodedLength,
            reinterpret_cast<const unsigned char *>(KEEPASS_SECRETS_BASE64), encodedLength);
        if (status != 0)
        {
            Serial.println("Unable to decode KeePass secrets.");
            return nullptr;
        }
        // Const input makes ArduinoJson copy strings before the temporary buffer is freed.
        const DeserializationError error = deserializeJson(
            secrets, reinterpret_cast<const char *>(decoded.get()), decodedLength);
        if (error || !secrets.is<JsonObject>())
        {
            Serial.println("Invalid KeePass secret registry.");
            return nullptr;
        }
        // Cache only successful loads so a later call can retry after a failure.
        loaded = true;
    }

    if (name == nullptr)
    {
        return nullptr;
    }
    // Read without modifying the registry or invalidating previously returned pointers.
    const JsonVariantConst value = secrets.as<JsonObjectConst>()[name];
    if (!value.is<const char *>())
    {
        Serial.printf("KeePass secret not found: %s\n", name);
        return nullptr;
    }
    const char *secret = value.as<const char *>();
    if (certificate)
    {
        // Validate PEM syntax, not trust or expiry; mbedTLS requires the trailing NUL byte.
        mbedtls_x509_crt parsedCertificate;
        mbedtls_x509_crt_init(&parsedCertificate);
        const int status = mbedtls_x509_crt_parse(
            &parsedCertificate, reinterpret_cast<const unsigned char *>(secret), std::strlen(secret) + 1);
        mbedtls_x509_crt_free(&parsedCertificate);
        if (status != 0)
        {
            Serial.printf("Invalid KeePass certificate: %s\n", name);
            return nullptr;
        }
    }
    return secret;
}