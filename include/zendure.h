#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <math.h>
#include <time.h>
#include "config.h"
#include "secrets.h"

extern double batteryPowerW;
extern int batterySoc;
extern int64_t batteryTimestamp;

inline void lirePuissanceBatterie()
{
    WiFiClient client;
    HTTPClient http;
    http.setReuse(false);
    const String url = String("http://") + ZENDURE_IP + "/properties/report";
    if (!http.begin(client, url))
    {
        Serial.println("Unable to initialize the Zendure request.");
        return;
    }

    http.setConnectTimeout(1000);
    http.setTimeout(BATTERY_READ_TIMEOUT_MS);
    http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
    http.addHeader("Accept", "application/json");
    const unsigned long requestStartedMs = millis();
    const int statusCode = http.GET();
    if (statusCode != HTTP_CODE_OK)
    {
        Serial.printf("Zendure request failed: code %d (%s) | Elapsed: %lu ms | Wi-Fi RSSI: %d dBm\n",
                      statusCode, statusCode < 0 ? HTTPClient::errorToString(statusCode).c_str() : "HTTP error",
                      millis() - requestStartedMs, static_cast<int>(WiFi.RSSI()));
        client.stop();
        http.end();
        return;
    }

    JsonDocument response;
    const DeserializationError error = deserializeJson(response, http.getString());
    http.end();
    if (error)
    {
        Serial.println("Invalid Zendure response: unable to parse JSON.");
        return;
    }

    const JsonVariantConst outputHomePower = response["properties"]["outputHomePower"];
    const JsonVariantConst gridInputPower = response["properties"]["gridInputPower"];
    if (!outputHomePower.is<double>() || !gridInputPower.is<double>())
    {
        Serial.println("Invalid Zendure response: missing or non-numeric power values.");
        return;
    }

    const JsonVariantConst soc = response["properties"]["electricLevel"];
    if (!soc.is<int>() || soc.as<int>() < 0 || soc.as<int>() > 100 ||
        !isfinite(outputHomePower.as<double>()) || !isfinite(gridInputPower.as<double>()))
    {
        Serial.println("Invalid Zendure SOC or power values.");
        return;
    }

    batteryPowerW = outputHomePower.as<double>() - gridInputPower.as<double>();
    batterySoc = soc.as<int>();
    batteryTimestamp = static_cast<int64_t>(time(nullptr));
    Serial.printf("Battery : %.0f W | SOC : %d%%\n", batteryPowerW, batterySoc);
}

inline bool envoyerConsigneBatterie(int setpointW)
{
    const char *const ZENDURE_SERIAL_NUMBER = get_secret("ZENDURE_SERIAL_NUMBER", false);
    if (ZENDURE_SERIAL_NUMBER == nullptr || ZENDURE_SERIAL_NUMBER[0] == '\0')
    {
        Serial.println("Zendure serial number unavailable. Battery setpoint skipped.");
        return false;
    }

    WiFiClient client;
    HTTPClient http;
    http.setReuse(false);
    const String url = String("http://") + ZENDURE_IP + "/properties/write";
    if (!http.begin(client, url))
    {
        Serial.println("Unable to initialize the battery setpoint request.");
        return false;
    }
    http.setConnectTimeout(1000);
    http.setTimeout(BATTERY_READ_TIMEOUT_MS);
    http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
    http.addHeader("Content-Type", "application/json");

    JsonDocument request;
    request["sn"] = ZENDURE_SERIAL_NUMBER;
    request["properties"]["acMode"] = setpointW < 0 ? 1 : 2;
    request["properties"]["inputLimit"] = setpointW < 0 ? -setpointW : 0;
    request["properties"]["outputLimit"] = setpointW > 0 ? setpointW : 0;
    request["properties"]["smartMode"] = 1;
    String body;
    serializeJson(request, body);
    const int statusCode = http.POST(body);
    const String response = http.getString();
    if (statusCode < 200 || statusCode >= 300)
    {
        Serial.printf("Battery setpoint failed: code %d (%s) | Response: %s\n",
                      statusCode, statusCode < 0 ? HTTPClient::errorToString(statusCode).c_str() : "HTTP error",
                      response.c_str());
        client.stop();
        http.end();
        return false;
    }
    http.end();
    Serial.printf("Battery setpoint : %d W\n", setpointW);
    return true;
}
