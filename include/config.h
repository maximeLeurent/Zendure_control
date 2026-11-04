#pragma once

#include <cstdint>

// --- Enphase: connection and measurements ---

// Enphase gateway IPv4 address (no unit).
// An empty string disables power readings.
const char *const ENPHASE_GATEWAY_IP = "192.168.1.4";

// Interval between power readings (ms).
constexpr unsigned long POWER_POLL_INTERVAL_MS = 1000;

// --- Enphase: authentication ---

// Time before expiration at which to refresh the token (s).
constexpr int64_t TOKEN_REFRESH_MARGIN_SECONDS = 60;

// Minimum delay between token retrieval attempts (ms).
constexpr unsigned long TOKEN_RETRY_INTERVAL_MS = 30000;

// --- Zendure battery: connection and measurements ---

// Zendure battery IPv4 address (no unit).
// An empty string disables battery readings and control.
const char *const ZENDURE_IP = "192.168.1.2";

// Interval between battery status readings (ms).
constexpr unsigned long BATTERY_POLL_INTERVAL_MS = 1000;

// Maximum wait time for a battery reading (ms).
constexpr uint16_t BATTERY_READ_TIMEOUT_MS = 3000;

// --- Battery: charge and discharge limits ---

// State of charge below which discharging is disabled (%).
constexpr int BATTERY_SOC_MIN = 7;

// State of charge above which charging is disabled (%).
constexpr int BATTERY_SOC_MAX = 95;

// Maximum allowed battery charging power (W).
constexpr int BATTERY_MAX_CHARGE_POWER_W = 1500;

// Maximum allowed battery discharging power (W).
constexpr int BATTERY_MAX_DISCHARGE_POWER_W = 1500;

// --- Control: regulation and measurement validity ---

// Maximum measurement age before forcing the setpoint to zero (s).
constexpr int64_t MEASUREMENT_MAX_AGE_SECONDS = 10;

// Power threshold around zero with no charging or discharging (W).
constexpr double POWER_DEADBAND_W = 50.0;

// Minimum interval between setpoint commands (ms).
constexpr unsigned long CONTROL_INTERVAL_MS = 1000;

// --- Clock ---

// Earliest timestamp accepted as a synchronized clock
// (seconds since January 1, 1970 at 00:00 UTC).
constexpr int64_t MIN_VALID_UNIX_TIME = 1700000000;
