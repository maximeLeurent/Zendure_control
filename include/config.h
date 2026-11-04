#pragma once

#include <cstdint>

const char *const ENPHASE_GATEWAY_IP = "192.168.1.4";
const char *const ZENDURE_IP = "192.168.1.2";

constexpr int BATTERY_SOC_MIN = 7;
constexpr int BATTERY_SOC_MAX = 95;
constexpr int BATTERY_MAX_CHARGE_POWER_W = 1500;
constexpr int BATTERY_MAX_DISCHARGE_POWER_W = 1500;
constexpr int64_t MEASUREMENT_MAX_AGE_SECONDS = 10;
constexpr double POWER_DEADBAND_W = 50.0;
constexpr unsigned long CONTROL_INTERVAL_MS = 1000;
constexpr int64_t TOKEN_REFRESH_MARGIN_SECONDS = 60;     // Margin before token expiration to consider it invalid, in seconds.
constexpr unsigned long TOKEN_RETRY_INTERVAL_MS = 30000; // Period for retrying token request. In milliseconds.
constexpr int64_t MIN_VALID_UNIX_TIME = 1700000000;

constexpr unsigned long POWER_POLL_INTERVAL_MS = 1000;
constexpr unsigned long BATTERY_POLL_INTERVAL_MS = 1000;
constexpr uint16_t BATTERY_READ_TIMEOUT_MS = 3000;
