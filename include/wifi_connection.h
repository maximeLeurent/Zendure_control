#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "secrets.h"

inline bool needsWifiReconnection()
{
    return WiFi.status() != WL_CONNECTED;
}

inline void waitForWifiConnection()
{
    while (needsWifiReconnection())
    {
        delay(1000);
        Serial.println("Connecting...");
    }
    Serial.println("Connected to Wi-Fi!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

inline void connectToWifi(const char *ssid, const char *password)
{
    Serial.println("\nConnecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    waitForWifiConnection();
}

inline void setupWifi()
{
    Serial.println("\nCollecting Wifi secrets...");
    const char *const wifiSsid = get_secret("WIFI_SSID", false);
    const char *const wifiPassword = get_secret("WIFI_PASSWORD", false);
    if (wifiSsid == nullptr || wifiPassword == nullptr)
    {
        Serial.println("Wi-Fi secrets unavailable. Startup stopped.");
        while (true)
        {
            delay(1000);
        }
    }
    connectToWifi(wifiSsid, wifiPassword);
}

inline void reconnectToWifi()
{
    Serial.println("Wi-Fi disconnected. Attempting to reconnect...");
    WiFi.reconnect();
    waitForWifiConnection();
}