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

void connect();

bool recupererToken()
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

void lirePuissanceConsommation()
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

int getMaxPuissance(bool discharge)
{
    if (discharge)
    {
        return batterySoc < BATTERY_SOC_MIN ? 0 : BATTERY_MAX_DISCHARGE_POWER_W;
    }
    return batterySoc > BATTERY_SOC_MAX ? 0 : BATTERY_MAX_CHARGE_POWER_W;
}

int calculerConsigneBatterie(int64_t now)
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
        return static_cast<int>(fmin(gridNoBatteryW, getMaxPuissance(true)));
    }
    if (gridNoBatteryW < -POWER_DEADBAND_W)
    {
        return -static_cast<int>(fmin(-gridNoBatteryW, getMaxPuissance(false)));
    }
    return 0;
}

constexpr bool TEST_CHARGE = false;

void testerChargeBatterie()
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
    envoyerConsigneBatterie(-consigneW);
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
    Serial.println("\nConnecting to Wi-Fi...");
    WiFi.begin(wifiSsid, wifiPassword);
    connect();
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

void connect()
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

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Wi-Fi disconnected. Attempting to reconnect...");
        connect();
    }
    if (TEST_CHARGE)
    {
        testerChargeBatterie();
        delay(1);
        return;
    }
    const int64_t now = static_cast<int64_t>(time(nullptr));
    const unsigned long nowMs = millis();
    const bool tokenNeedsRefresh = enphaseToken.isEmpty() || enphaseTokenDeadline - now <= TOKEN_REFRESH_MARGIN_SECONDS;
    const bool retryAllowed = !tokenRequestAttempted || nowMs - lastTokenAttemptMs >= TOKEN_RETRY_INTERVAL_MS;
    if (now >= MIN_VALID_UNIX_TIME && tokenNeedsRefresh && retryAllowed)
    {
        recupererToken();
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
        lirePuissanceConsommation();
    }
    const unsigned long batteryNowMs = millis();
    if (ZENDURE_IP[0] != '\0' &&
        (!batteryRequestAttempted || batteryNowMs - lastBatteryRequestMs >= BATTERY_POLL_INTERVAL_MS))
    {
        lastBatteryRequestMs = batteryNowMs;
        batteryRequestAttempted = true;
        lirePuissanceBatterie();
    }
    const unsigned long controlNowMs = millis();
    if (ZENDURE_IP[0] != '\0' &&
        (!controlAttempted || controlNowMs - lastControlMs >= CONTROL_INTERVAL_MS))
    {
        envoyerConsigneBatterie(calculerConsigneBatterie(static_cast<int64_t>(time(nullptr))));
        lastControlMs = millis();
        controlAttempted = true;
    }
    delay(1);
}
