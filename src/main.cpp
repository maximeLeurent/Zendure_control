#include <Arduino.h>
#include <WiFi.h>
#include <math.h>
#include <time.h>

#include "config.h"
#include "enphase.h"
#include "secrets.h"
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

String enphaseToken;
int64_t enphaseTokenDeadline = 0;
unsigned long lastTokenAttemptMs = 0;
bool tokenRequestAttempted = false;

int getMaxBatteryPower(bool discharge)
{
    if (discharge)
    {
        return batterySoc < BATTERY_SOC_MIN ? 0 : BATTERY_MAX_DISCHARGE_POWER_W;
    }
    return batterySoc > BATTERY_SOC_MAX ? 0 : BATTERY_MAX_CHARGE_POWER_W;
}

int calculateBatterySetpoint(int64_t now)
{
    if (now < MIN_VALID_UNIX_TIME || consumptionTimestamp == 0 || batteryTimestamp == 0 ||
        now < consumptionTimestamp || now < batteryTimestamp)
    {
        return 0;
    }

    const int64_t consumptionAgeSeconds = now - consumptionTimestamp;
    const int64_t batteryAgeSeconds = now - batteryTimestamp;
    if (consumptionAgeSeconds > MEASUREMENT_MAX_AGE_SECONDS)
    {
        Serial.printf("Stale consumption measurement: %lld s old (limit: %lld s). Battery setpoint forced to 0 W.\n",
                      static_cast<long long>(consumptionAgeSeconds),
                      static_cast<long long>(MEASUREMENT_MAX_AGE_SECONDS));
    }
    if (batteryAgeSeconds > MEASUREMENT_MAX_AGE_SECONDS)
    {
        Serial.printf("Stale battery measurement: %lld s old (limit: %lld s). Battery setpoint forced to 0 W.\n",
                      static_cast<long long>(batteryAgeSeconds),
                      static_cast<long long>(MEASUREMENT_MAX_AGE_SECONDS));
    }
    if (consumptionAgeSeconds > MEASUREMENT_MAX_AGE_SECONDS || batteryAgeSeconds > MEASUREMENT_MAX_AGE_SECONDS)
    {
        return 0;
    }

    const double gridNoBatteryW = consumptionPowerW + batteryPowerW;
    Serial.printf("Net consumption without battery : %.0f W\n", gridNoBatteryW);
    if (gridNoBatteryW > POWER_DEADBAND_W)
    {
        return static_cast<int>(fmin(gridNoBatteryW, getMaxBatteryPower(true)));
    }
    if (gridNoBatteryW < -POWER_DEADBAND_W)
    {
        return -static_cast<int>(fmin(-gridNoBatteryW, getMaxBatteryPower(false)));
    }
    return 0;
}

constexpr bool TEST_CHARGE = false;

void testBatteryCharging()
{
    static int consigneW = 50;
    static unsigned long dernierEnvoiMs = 0;
    static bool envoiEffectue = false;
    const unsigned long nowMs = millis();
    if (envoiEffectue && nowMs - dernierEnvoiMs < 5000)
    {
        return;
    }

    dernierEnvoiMs = nowMs;
    envoiEffectue = true;
    sendBatterySetpoint(-consigneW);
    consigneW = consigneW >= 300 ? 50 : consigneW + 50;
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(1000);
    if (ENPHASE_GATEWAY_IP[0] == '\0')
    {
        Serial.println("Set ENPHASE_GATEWAY_IP to the local gateway IP address to enable power readings.");
    }
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
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        reconnectToWifi();
    }
    if (TEST_CHARGE)
    {
        testBatteryCharging();
        delay(1);
        return;
    }
    const int64_t now = static_cast<int64_t>(time(nullptr));
    const unsigned long nowMs = millis();
    const bool tokenNeedsRefresh = enphaseToken.isEmpty() || enphaseTokenDeadline - now <= TOKEN_REFRESH_MARGIN_SECONDS;
    const bool retryAllowed = !tokenRequestAttempted || nowMs - lastTokenAttemptMs >= TOKEN_RETRY_INTERVAL_MS;
    if (now >= MIN_VALID_UNIX_TIME && tokenNeedsRefresh && retryAllowed)
    {
        retrieveEnphaseToken();
        lastTokenAttemptMs = millis();
        tokenRequestAttempted = true;
    }
    const unsigned long powerNowMs = millis();
    if (ENPHASE_GATEWAY_IP[0] != '\0' &&
        (!powerRequestAttempted || powerNowMs - lastPowerRequestMs >= POWER_POLL_INTERVAL_MS) &&
        !enphaseToken.isEmpty() && enphaseTokenDeadline > static_cast<int64_t>(time(nullptr)))
    {
        lastPowerRequestMs = powerNowMs;
        powerRequestAttempted = true;
        readEnphaseConsumptionPower();
    }
    const unsigned long batteryNowMs = millis();
    if (ZENDURE_IP[0] != '\0' &&
        (!batteryRequestAttempted || batteryNowMs - lastBatteryRequestMs >= BATTERY_POLL_INTERVAL_MS))
    {
        lastBatteryRequestMs = batteryNowMs;
        batteryRequestAttempted = true;
        readBatteryStatus();
    }
    const unsigned long controlNowMs = millis();
    if (ZENDURE_IP[0] != '\0' &&
        (!controlAttempted || controlNowMs - lastControlMs >= CONTROL_INTERVAL_MS))
    {
        sendBatterySetpoint(calculateBatterySetpoint(static_cast<int64_t>(time(nullptr))));
        lastControlMs = millis();
        controlAttempted = true;
    }
    delay(1);
}
