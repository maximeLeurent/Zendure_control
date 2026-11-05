#include <Arduino.h>
#include <time.h>

#include "config.h"
#include "control.h"
#include "enphase.h"
#include "wifi_connection.h"
#include "zendure.h"


double consumptionPowerW = 0;
double batteryPowerW = 0;
int batterySoc = 0;
int64_t consumptionTimestamp = 0;
int64_t batteryTimestamp = 0;
unsigned long lastControlMs = 0;
bool controlAttempted = false;

unsigned long lastPowerRequestMs = 0;
bool powerRequestAttempted = false;
unsigned long lastBatteryRequestMs = 0;
bool batteryRequestAttempted = false;

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(1000);
    verifyEnphaseParameters();
    setupWifi();
    configTzTime(LOCAL_TIME_ZONE, "pool.ntp.org", "time.nist.gov");
}

void loop()
{
    if (needsWifiReconnection())
    {
        reconnectToWifi();
    }
    runControl();
    delay(1);
}
