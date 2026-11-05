#include <assert.h>
#include <stdio.h>

#include "battery_strategy.h"

tm makeLocalTime(int month, int day, int hour = 0, int minute = 0)
{
    tm result = {};
    result.tm_year = 2026 - 1900;
    result.tm_mon = month - 1;
    result.tm_mday = day;
    result.tm_hour = hour;
    result.tm_min = minute;
    result.tm_isdst = -1;
    return result;
}

int main()
{
    assert(!isWinterDate(makeLocalTime(9, 30)));
    assert(isWinterDate(makeLocalTime(10, 1)));
    assert(isWinterDate(makeLocalTime(12, 31)));
    assert(isWinterDate(makeLocalTime(1, 1)));
    assert(isWinterDate(makeLocalTime(3, 31)));
    assert(!isWinterDate(makeLocalTime(4, 1)));

    assert(!isOffPeakTime(makeLocalTime(1, 15, 22, 29)));
    assert(isOffPeakTime(makeLocalTime(1, 15, 22, 30)));
    assert(isOffPeakTime(makeLocalTime(1, 16, 0, 0)));
    assert(isOffPeakTime(makeLocalTime(1, 16, 6, 29)));
    assert(!isOffPeakTime(makeLocalTime(1, 16, 6, 30)));

    tm evening = makeLocalTime(1, 15, 22, 30);
    const time_t eveningTime = mktime(&evening);
    assert(getOffPeakSecondsRemaining(eveningTime, evening) == 8 * 3600);
    tm midnight = makeLocalTime(1, 16);
    const time_t midnightTime = mktime(&midnight);
    assert(getOffPeakSecondsRemaining(midnightTime, midnight) == 6 * 3600 + 30 * 60);
    tm morning = makeLocalTime(1, 16, 6, 30);
    const time_t morningTime = mktime(&morning);
    assert(getOffPeakSecondsRemaining(morningTime, morning) == 0);

    assert(calculateSeasonalBatteryPower(500, 70, true, 8 * 3600) == -78);
    assert(calculateSeasonalBatteryPower(500, 70, true, 3600) == -1162);
    assert(calculateSeasonalBatteryPower(500, 70, true, 1800) == -1500);
    assert(calculateSeasonalBatteryPower(500, 70, true, 1) == -1500);
    assert(calculateSeasonalBatteryPower(500, 80, true, 8 * 3600) == 0);
    assert(calculateSeasonalBatteryPower(500, 90, true, 8 * 3600) == 0);
    assert(calculateSeasonalBatteryPower(-1000, 70, true, 8 * 3600) == -1000);
    assert(calculateSeasonalBatteryPower(-1000, 85, true, 8 * 3600) == -1000);
    assert(calculateSeasonalBatteryPower(-2000, 85, true, 0) == -1500);
    assert(calculateSeasonalBatteryPower(-500, 95, true, 8 * 3600) == 0);

    assert(calculateSeasonalBatteryPower(500, 70, true, 0) == 500);
    assert(calculateSeasonalBatteryPower(2000, 70, true, 0) == 1500);
    assert(calculateSeasonalBatteryPower(500, 7, true, 0) == 0);
    assert(calculateSeasonalBatteryPower(500, 6, true, 0) == 0);
    assert(calculateSeasonalBatteryPower(-500, 7, true, 0) == -500);

    assert(calculateSeasonalBatteryPower(500, 70, false, 8 * 3600) == 500);
    assert(calculateSeasonalBatteryPower(-500, 85, false, 8 * 3600) == -500);
    assert(calculateSeasonalBatteryPower(-500, 95, false, 0) == 0);
    assert(calculateSeasonalBatteryPower(50, 70, false, 0) == 0);
    assert(calculateSeasonalBatteryPower(-50, 70, false, 0) == 0);
    assert(calculateSeasonalBatteryPower(0, 70, true, 0) == 0);

    assert(calculateSeasonalBatteryPower(5000, 70, true, 1800) == -1000);
    assert(calculateSeasonalBatteryPower(5999.5, 70, true, 1800) == 0);
    assert(calculateSeasonalBatteryPower(6000, 70, true, 1800) == 0);
    assert(calculateSeasonalBatteryPower(6000.1, 70, true, 1800) == 1);
    assert(calculateSeasonalBatteryPower(6500, 70, true, 1800) == 500);
    assert(calculateSeasonalBatteryPower(6500, 80, true, 1800) == 500);
    assert(calculateSeasonalBatteryPower(7500, 70, true, 1800) == 1500);
    assert(calculateSeasonalBatteryPower(8000, 70, true, 1800) == 1500);
    assert(calculateSeasonalBatteryPower(6500, 7, true, 1800) == 0);
    assert(calculateSeasonalBatteryPower(6500, 6, true, 1800) == 0);
    assert(calculateSeasonalBatteryPower(5900, 7, true, 1800) == -100);
    assert(calculateSeasonalBatteryPower(6500, 95, true, 1800) == 500);
    assert(calculateSeasonalBatteryPower(6500, 70, false, 1800) == 1500);
    assert(calculateSeasonalBatteryPower(6500, 7, false, 0) == 0);
    assert(limitGridImportPower(5800, 70, -1000) == -200);
    assert(limitGridImportPower(6500, 70, 800) == 800);

    puts("Battery strategy tests passed.");
    return 0;
}