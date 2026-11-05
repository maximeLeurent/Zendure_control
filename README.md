# Zendure Control

ESP32 firmware that adjusts a local Zendure battery's charge or discharge power using net-consumption measurements from an Enphase gateway.

The firmware runs on an ESP32 with the Arduino framework. It polls both devices over the local network and sends a battery setpoint approximately once per second. It also obtains an Enphase Enlighten token to read the gateway's consumption endpoint.

## How it works

The ESP32 reads consumption data from the Enphase gateway and battery data from the Zendure device, then sends a power request to the battery. Local dates and tariff times use Paris time with automatic daylight-saving changes.

- **Winter (October 1 through March 31, inclusive):** during off-peak hours (22:30 to 06:30), charge toward 80% SOC without discharging. Charge power is recalculated from missing energy and time remaining, aiming to finish at 06:00 (30 minutes early). Outside off-peak hours, discharge only to cover net imports, down to the minimum SOC.
- **Summer (the remaining dates):** charge from PV surplus and discharge to cover net imports, irrespective of tariff times.
- **All year:** PV surplus is absorbed up to the maximum SOC, including above the winter 80% target. During winter off-peak hours, charging uses the greater of the surplus and the calculated charging power. Charge and discharge stop at their respective SOC limits; power limits, the deadband, and measurement freshness checks remain active.

The charging estimate uses 5,810 Wh of storage capacity. The early-finish margin provides some allowance for losses and SOC rounding, but reaching the target is not guaranteed if available power or time is insufficient. If the target is still unmet after 06:00, charging continues at the maximum allowed power until 06:30 or until the target is reached. All dates, times, capacity, margin, and the winter target are configurable.

Grid import limiting has priority over the seasonal strategy. Using measured net grid import plus current battery power, the controller reconstructs demand without the battery and adjusts the final setpoint to keep grid import at or below 6,000 W: it reduces charging, stops charging, or forces discharge even during winter off-peak hours. Battery minimum SOC and maximum discharge power remain enforced. If they prevent meeting the import limit, the controller requests the best permitted setpoint and logs a warning. This is a sampled software control, not a hard electrical protection: measurement delays, communication failures, sudden load changes, or insufficient battery reserves can still cause an exceedance. Stale measurement safety still forces a zero setpoint.

The firmware currently uses the Zendure local HTTP endpoints `/properties/report` and `/properties/write`.

## Hardware and network requirements

- An ESP32 development board supported by PlatformIO's `esp32dev` target.
- A Zendure device whose local API provides the properties used by this firmware.
- An Enphase gateway exposing its local HTTPS API, plus an Enlighten account and system serial number.
- The ESP32, Enphase gateway, and Zendure device must be reachable on the same local network. The ESP32 also needs internet access for NTP and Enphase authentication.
- PlatformIO Core or the PlatformIO IDE, Python 3, KeePassXC, and `keepassxc-cli` available on `PATH`.

## Configuration

### Device and control settings

Edit `include/config.h` to match your network and battery. The checked-in IP addresses are examples for the original installation and are not intended to match every network.

| Setting | Purpose |
| --- | --- |
| `ENPHASE_GATEWAY_IP` | Enphase gateway address |
| `ZENDURE_IP` | Zendure device address |
| `BATTERY_SOC_MIN` / `BATTERY_SOC_MAX` | SOC limits for discharge / charge |
| `BATTERY_MAX_CHARGE_POWER_W` | Maximum requested charge power |
| `BATTERY_MAX_DISCHARGE_POWER_W` | Maximum requested discharge power |
| `POWER_DEADBAND_W` | Range around zero where the request is zero |
| `MEASUREMENT_MAX_AGE_SECONDS` | Maximum accepted age of either measurement |
| `GRID_MAX_IMPORT_POWER_W` | Priority limit on grid import (6,000 W), subject to battery capability |
| `BATTERY_CAPACITY_WH` | Total usable battery capacity for the charging estimate (5,810 Wh) |
| `WINTER_START_MONTH` / `WINTER_START_DAY` | First winter date, inclusive (October 1) |
| `WINTER_END_MONTH` / `WINTER_END_DAY` | Last winter date, inclusive (March 31) |
| `WINTER_CHARGE_TARGET_SOC` | Winter off-peak grid-charging target (80%) |
| `OFF_PEAK_START_MINUTE` / `OFF_PEAK_END_MINUTE` | Daily local off-peak interval, minutes after midnight (22:30 to 06:30); supports crossing midnight |
| `WINTER_CHARGE_MARGIN_SECONDS` | How early to aim to reach the target (1,800 s) |
| `LOCAL_TIME_ZONE` | POSIX local timezone rule (Paris, with daylight saving) |

Set power limits and SOC thresholds to values supported by your specific battery and installation.

### Secrets with KeePassXC

Secrets are injected into the firmware at build time by `load_secrets.py`; do not put credentials in source files or commit them. Create KeePassXC entries with the following exact names, and store each value in the entry's **Password** field:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `ENPHASE_SERIAL_NUMBER`
- `ENPHASE_USERNAME`
- `ENPHASE_PASSWORD`
- `ENPHASE_ROOT_CA`
- `ZENDURE_SERIAL_NUMBER`

`ENPHASE_ROOT_CA` must contain the complete PEM CA certificate or bundle, including the `BEGIN CERTIFICATE` and `END CERTIFICATE` lines. Do not wrap it in a C++ raw string. The build script normalizes line endings and validates the certificate format.

Before building, update `KEEPASS_DB` in `load_secrets.py` to the path of your KeePass database. The script prompts for the KeePass master password, reads the entries using `keepassxc-cli`, and passes the encoded secret registry to the compiler without writing the retrieved secrets to a file. KeePassXC and its CLI must be installed, and `keepassxc-cli` must be on `PATH`.

## Build, upload, and monitor

From the project directory, use the PlatformIO CLI:

```sh
pio run
pio run --target upload
pio device monitor --baud 115200
```

The build invokes the KeePass pre-build script and prompts for the database master password. Select the appropriate serial upload port in PlatformIO if it cannot be detected automatically. The serial monitor reports connection status, measurements, token requests, and battery setpoints.

## Test-charge mode

### Strategy tests

The standalone C++ tests do not require hardware, Arduino libraries, or secrets. With a native C++ compiler, run from the project directory:

```sh
g++ -std=c++11 -Iinclude test/test_battery_strategy.cpp -o test_battery_strategy.exe
./test_battery_strategy.exe
```

They cover season boundaries, the overnight off-peak interval, adaptive charging, PV priority, summer behavior, power limits, inclusive SOC limits, and priority grid import limiting (including fractional watts and insufficient battery capability).

### Hardware test-charge mode

`TEST_CHARGE` in `include/control.h` is a compile-time switch and is `false` by default. When enabled, it bypasses normal regulation and sends charge requests that step from 50 W to 300 W in 50 W increments, approximately every five seconds. Use this only when deliberately testing the battery in a controlled setup, and restore it to `false` for normal operation.

## Security and operational notes

- The Enlighten login/token requests validate TLS using `ENPHASE_ROOT_CA`.
- The Enphase gateway HTTPS client currently calls `setInsecure()`, so it does **not** validate the gateway's TLS certificate. This should be addressed before using the firmware on an untrusted network.
- Zendure local API requests use plain HTTP. Keep the device and ESP32 on a trusted, access-controlled local network.
- This firmware can actively control battery power. Verify IP addresses, power limits, SOC thresholds, device compatibility, and the behavior of your installation before enabling unattended operation.
- If a measurement is stale or unavailable, the calculated setpoint is zero. Check the serial output if the battery repeatedly receives zero-power requests.

## Project layout

| Path | Contents |
| --- | --- |
| `src/main.cpp` | Startup, Wi-Fi reconnection, and main loop |
| `include/control.h` | Token refresh scheduling, polling, control calculation, and test-charge mode |
| `include/enphase.h` | Enphase Enlighten token retrieval and gateway consumption reads |
| `src/enphase.cpp` | Shared Enphase token and retry state |
| `include/zendure.h` | Zendure local API reads and setpoint writes |
| `include/wifi_connection.h` | Initial Wi-Fi connection and reconnection helpers |
| `include/config.h` | Network addresses and control parameters |
| `include/secrets.h` | Runtime accessor for the build-time injected KeePass secret registry |
| `load_secrets.py` | PlatformIO pre-build script that retrieves and injects KeePass secrets |
| `platformio.ini` | ESP32/Arduino build configuration and ArduinoJson dependency |

## Troubleshooting

- **KeePass entry not found:** Check that the entry name matches the list above and that the secret is in its Password field.
- **KeePass database cannot be opened:** Check `KEEPASS_DB`, confirm KeePassXC is installed, and verify `keepassxc-cli` is on `PATH`.
- **Enphase token request fails:** Confirm Wi-Fi/internet access, NTP synchronization, Enlighten credentials, system serial number, and the CA certificate.
- **Gateway or Zendure request fails:** Confirm the configured IP addresses, local network reachability, and that each device's local API is enabled and compatible.
- **Setpoint stays at zero:** Check that both devices return valid measurements, that timestamps are current, and that the battery SOC is within the configured thresholds.