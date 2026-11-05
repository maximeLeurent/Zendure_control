#pragma once

#include <math.h>
#include <time.h>

#include "config.h"

static_assert(BATTERY_CAPACITY_WH > 0, "Battery capacity must be positive.");
static_assert(WINTER_CHARGE_TARGET_SOC >= BATTERY_SOC_MIN && WINTER_CHARGE_TARGET_SOC <= BATTERY_SOC_MAX,
              "Winter target must be within battery SOC limits.");
static_assert(OFF_PEAK_START_MINUTE >= 0 && OFF_PEAK_START_MINUTE < 24 * 60 &&
                  OFF_PEAK_END_MINUTE >= 0 && OFF_PEAK_END_MINUTE < 24 * 60 &&
                  OFF_PEAK_START_MINUTE != OFF_PEAK_END_MINUTE,
              "Off-peak times must define a nonempty interval within a day.");
static_assert(WINTER_CHARGE_MARGIN_SECONDS >= 0, "Charging margin must not be negative.");
static_assert(GRID_MAX_IMPORT_POWER_W > 0, "Grid import limit must be positive.");

inline bool isWinterDate(const tm &localTime)
{
    const int date = (localTime.tm_mon + 1) * 100 + localTime.tm_mday;
    const int start = WINTER_START_MONTH * 100 + WINTER_START_DAY;
    const int end = WINTER_END_MONTH * 100 + WINTER_END_DAY;
    return start <= end ? date >= start && date <= end : date >= start || date <= end;
}

inline bool isOffPeakTime(const tm &localTime)
{
    const int minute = localTime.tm_hour * 60 + localTime.tm_min;
    return OFF_PEAK_START_MINUTE < OFF_PEAK_END_MINUTE
               ? minute >= OFF_PEAK_START_MINUTE && minute < OFF_PEAK_END_MINUTE
               : minute >= OFF_PEAK_START_MINUTE || minute < OFF_PEAK_END_MINUTE;
}

inline int64_t getOffPeakSecondsRemaining(time_t now, const tm &localTime)
{
    if (!isOffPeakTime(localTime))
    {
        return 0;
    }

    tm end = localTime;
    if (OFF_PEAK_START_MINUTE > OFF_PEAK_END_MINUTE &&
        localTime.tm_hour * 60 + localTime.tm_min >= OFF_PEAK_START_MINUTE)
    {
        end.tm_mday += 1;
    }
    end.tm_hour = OFF_PEAK_END_MINUTE / 60;
    end.tm_min = OFF_PEAK_END_MINUTE % 60;
    end.tm_sec = 0;
    end.tm_isdst = -1;
    const time_t endTime = mktime(&end);
    return endTime == static_cast<time_t>(-1) ? 0 : static_cast<int64_t>(difftime(endTime, now));
}

inline int calculateUnconstrainedSeasonalBatteryPower(double gridNoBatteryW, int soc, bool winter,
                                                      int64_t offPeakSecondsRemaining)
{
    const int chargeLimitW = soc >= BATTERY_SOC_MAX ? 0 : BATTERY_MAX_CHARGE_POWER_W;
    const double surplusW = gridNoBatteryW < -POWER_DEADBAND_W ? -gridNoBatteryW : 0;
    double chargePowerW = fmin(surplusW, chargeLimitW);

    if (winter && offPeakSecondsRemaining > 0)
    {
        if (soc < WINTER_CHARGE_TARGET_SOC)
        {
            const double missingEnergyWh = BATTERY_CAPACITY_WH * (WINTER_CHARGE_TARGET_SOC - soc) / 100.0;
            const int64_t chargingSeconds = offPeakSecondsRemaining - WINTER_CHARGE_MARGIN_SECONDS;
            const double scheduledPowerW = ceil(missingEnergyWh * 3600.0 / (chargingSeconds > 0 ? chargingSeconds : 1));
            chargePowerW = fmin(fmax(chargePowerW, scheduledPowerW), chargeLimitW);
        }
        return -static_cast<int>(chargePowerW);
    }

    if (chargePowerW > 0)
    {
        return -static_cast<int>(chargePowerW);
    }
    if (gridNoBatteryW > POWER_DEADBAND_W && soc > BATTERY_SOC_MIN)
    {
        return static_cast<int>(fmin(gridNoBatteryW, BATTERY_MAX_DISCHARGE_POWER_W));
    }
    return 0;
}

inline int limitGridImportPower(double gridNoBatteryW, int soc, int requestedBatteryPowerW)
{
    const int dischargeLimitW = soc <= BATTERY_SOC_MIN ? 0 : BATTERY_MAX_DISCHARGE_POWER_W;
    const double minimumBatteryPowerW = ceil(gridNoBatteryW - GRID_MAX_IMPORT_POWER_W);
    return static_cast<int>(fmax(requestedBatteryPowerW, fmin(minimumBatteryPowerW, dischargeLimitW)));
}

inline int calculateSeasonalBatteryPower(double gridNoBatteryW, int soc, bool winter,
                                         int64_t offPeakSecondsRemaining)
{
    const int requestedBatteryPowerW =
        calculateUnconstrainedSeasonalBatteryPower(gridNoBatteryW, soc, winter, offPeakSecondsRemaining);
    return limitGridImportPower(gridNoBatteryW, soc, requestedBatteryPowerW);
}