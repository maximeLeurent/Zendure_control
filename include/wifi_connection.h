#pragma once

#include <Arduino.h>
#include <WiFi.h>

inline void waitForWifiConnection()
{
    while (WiFi.status() != WL_CONNECTED)
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

inline void reconnectToWifi()
{
    Serial.println("Wi-Fi disconnected. Attempting to reconnect...");
    WiFi.reconnect();
    waitForWifiConnection();
}