#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <mbedtls/base64.h>
#include <math.h>
#include <time.h>

#include "config.h"
#include "secrets.h"

extern double consumptionPowerW;
extern int64_t consumptionTimestamp;
extern String enphaseToken;
extern int64_t enphaseTokenDeadline;

inline bool recupererToken()
{
    Serial.println("\nStarting token retrieval...");
    if (WiFi.status() != WL_CONNECTED || time(nullptr) < MIN_VALID_UNIX_TIME)
    {
        return false;
    }
    Serial.println("\nCollecting Enphase secrets...");
    const char *const ENPHASE_SERIAL_NUMBER = get_secret("ENPHASE_SERIAL_NUMBER", false);
    const char *const ENPHASE_USERNAME = get_secret("ENPHASE_USERNAME", false);
    const char *const ENPHASE_PASSWORD = get_secret("ENPHASE_PASSWORD", false);
    const char *const ENPHASE_ROOT_CA = get_secret("ENPHASE_ROOT_CA", true);
    if (ENPHASE_SERIAL_NUMBER == nullptr || ENPHASE_USERNAME == nullptr ||
        ENPHASE_PASSWORD == nullptr || ENPHASE_ROOT_CA == nullptr)
    {
        Serial.println("Enphase secrets unavailable. Token request skipped.");
        return false;
    }
    if (ENPHASE_SERIAL_NUMBER[0] == '\0' || ENPHASE_USERNAME[0] == '\0' || ENPHASE_PASSWORD[0] == '\0')
    {
        Serial.println("Set ENPHASE_SERIAL_NUMBER, ENPHASE_USERNAME and ENPHASE_PASSWORD in KeePass.");
        return false;
    }

    WiFiClientSecure client;
    client.setCACert(ENPHASE_ROOT_CA);
    HTTPClient http;
    Serial.printf("[Enphase][Login] POST https://enlighten.enphaseenergy.com/login/login.json | Wi-Fi RSSI: %d dBm\n",
                  static_cast<int>(WiFi.RSSI()));
    if (!http.begin(client, "https://enlighten.enphaseenergy.com/login/login.json"))
    {
        Serial.println("Unable to initialize the Enlighten login request.");
        return false;
    }

    http.setConnectTimeout(10000);
    http.setTimeout(10000);
    http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
    http.addHeader("Accept", "application/json");
    const String boundary = "----ESP32EnphaseLogin";
    http.addHeader("Content-Type", String("multipart/form-data; boundary=") + boundary);
    const String loginBody = String("--") + boundary +
                             "\r\nContent-Disposition: form-data; name=\"user[email]\"\r\n\r\n" +
                             ENPHASE_USERNAME + "\r\n--" + boundary +
                             "\r\nContent-Disposition: form-data; name=\"user[password]\"\r\n\r\n" +
                             ENPHASE_PASSWORD + "\r\n--" + boundary + "--\r\n";
    const unsigned long loginStartedMs = millis();
    const int loginStatus = http.POST(loginBody);
    Serial.printf("[Enphase][Login] Result: %d | Elapsed: %lu ms\n", loginStatus, millis() - loginStartedMs);
    if (loginStatus != HTTP_CODE_OK)
    {
        Serial.printf("Enlighten login failed: code %d (%s).\n", loginStatus,
                      loginStatus < 0 ? HTTPClient::errorToString(loginStatus).c_str() : "HTTP error");
        http.end();
        return false;
    }

    JsonDocument loginResponse;
    const DeserializationError loginError = deserializeJson(loginResponse, http.getString());
    http.end();
    if (loginError || !loginResponse["session_id"].is<String>())
    {
        Serial.println("Invalid Enlighten login response: missing session_id.");
        return false;
    }
    const String sessionId = loginResponse["session_id"].as<String>();
    if (sessionId.isEmpty())
    {
        Serial.println("Enlighten login returned an empty session_id.");
        return false;
    }

    Serial.println("[Enphase][Login] Session received.");
    Serial.println("[Enphase][Token] POST https://entrez.enphaseenergy.com/tokens");
    if (!http.begin(client, "https://entrez.enphaseenergy.com/tokens"))
    {
        Serial.println("Unable to initialize the Enphase token request.");
        return false;
    }
    http.setConnectTimeout(10000);
    http.setTimeout(10000);
    http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
    http.addHeader("Content-Type", "application/json");
    JsonDocument tokenRequest;
    tokenRequest["session_id"] = sessionId;
    tokenRequest["serial_num"] = ENPHASE_SERIAL_NUMBER;
    tokenRequest["username"] = ENPHASE_USERNAME;
    String tokenBody;
    serializeJson(tokenRequest, tokenBody);
    const unsigned long tokenStartedMs = millis();
    const int statusCode = http.POST(tokenBody);
    Serial.printf("[Enphase][Token] Result: %d | Elapsed: %lu ms\n", statusCode, millis() - tokenStartedMs);
    if (statusCode != HTTP_CODE_OK)
    {
        Serial.printf("Token retrieval failed: code %d (%s).\n", statusCode,
                      statusCode < 0 ? HTTPClient::errorToString(statusCode).c_str() : "HTTP error; check the Enlighten session");
        http.end();
        return false;
    }

    String newToken = http.getString();
    http.end();
    newToken.trim();
    const int firstDot = newToken.indexOf('.');
    const int secondDot = newToken.indexOf('.', firstDot + 1);
    if (firstDot <= 0 || secondDot <= firstDot + 1 || secondDot >= static_cast<int>(newToken.length()) - 1)
    {
        Serial.println("Invalid Enphase response: expected a JWT token.");
        return false;
    }

    String encodedPayload = newToken.substring(firstDot + 1, secondDot);
    encodedPayload.replace('-', '+');
    encodedPayload.replace('_', '/');
    while (encodedPayload.length() % 4 != 0)
    {
        encodedPayload += '=';
    }
    String decodedPayload;
    if (!decodedPayload.reserve(encodedPayload.length()))
    {
        Serial.println("Unable to allocate memory for the token payload.");
        return false;
    }
    for (size_t index = 0; index < encodedPayload.length(); ++index)
    {
        decodedPayload += ' ';
    }
    size_t decodedLength = 0;
    const int decodeStatus = mbedtls_base64_decode(
        reinterpret_cast<unsigned char *>(&decodedPayload[0]), decodedPayload.length(), &decodedLength,
        reinterpret_cast<const unsigned char *>(encodedPayload.c_str()), encodedPayload.length());
    JsonDocument payload;
    if (decodeStatus != 0 || deserializeJson(payload, decodedPayload.c_str(), decodedLength) || !payload["exp"].is<int64_t>())
    {
        Serial.println("Invalid token payload: missing expiration.");
        return false;
    }
    const int64_t newDeadline = payload["exp"].as<int64_t>();
    if (newToken.isEmpty() || newDeadline <= static_cast<int64_t>(time(nullptr)) + TOKEN_REFRESH_MARGIN_SECONDS)
    {
        Serial.println("Received token is empty or too close to expiration.");
        return false;
    }

    enphaseToken = newToken;
    enphaseTokenDeadline = newDeadline;
    const int64_t remainingSeconds = enphaseTokenDeadline - static_cast<int64_t>(time(nullptr));
    Serial.printf("Received token is valid for %lld days %lld hours %lld minutes %lld seconds.\n",
                  static_cast<long long>(remainingSeconds / 86400),
                  static_cast<long long>((remainingSeconds % 86400) / 3600),
                  static_cast<long long>((remainingSeconds % 3600) / 60),
                  static_cast<long long>(remainingSeconds % 60));
    Serial.printf("Enphase token retrieved. Unix expiration: %lld\n", static_cast<long long>(enphaseTokenDeadline));
    return true;
}

inline void lirePuissanceConsommation()
{
    static WiFiClientSecure client;
    static HTTPClient http;
    client.setInsecure();
    http.setReuse(true);
    const String url = String("https://") + ENPHASE_GATEWAY_IP + "/ivp/meters/reports/consumption";
    if (!http.begin(client, url))
    {
        Serial.println("Unable to initialize the gateway request.");
        return;
    }

    http.setConnectTimeout(10000);
    http.setTimeout(10000);
    http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
    http.addHeader("Accept", "application/json");
    http.addHeader("Authorization", String("Bearer ") + enphaseToken);
    const int statusCode = http.GET();
    if (statusCode != HTTP_CODE_OK)
    {
        Serial.printf("Consumption request failed: HTTP %d (%s).\n",
                      statusCode, statusCode < 0 ? HTTPClient::errorToString(statusCode).c_str() : "HTTP error");
        client.stop();
        http.end();
        return;
    }

    JsonDocument response;
    const DeserializationError error = deserializeJson(response, http.getString());
    http.end();
    if (error || !response.is<JsonArray>())
    {
        Serial.println("Invalid consumption response: expected a JSON array.");
        return;
    }

    for (JsonObjectConst report : response.as<JsonArrayConst>())
    {
        if (report["reportType"] == "net-consumption")
        {
            const JsonVariantConst activePower = report["cumulative"]["actPower"];
            if (!activePower.is<double>())
            {
                Serial.println("Invalid net-consumption report: missing or non-numeric actPower.");
                return;
            }
            if (!isfinite(activePower.as<double>()))
            {
                Serial.println("Invalid consumption power.");
                return;
            }
            consumptionPowerW = activePower.as<double>();
            consumptionTimestamp = static_cast<int64_t>(time(nullptr));
            Serial.printf("Net consumption : %.0f W\n", consumptionPowerW);
            return;
        }
    }
    Serial.println("Consumption response is missing the net-consumption report.");
}