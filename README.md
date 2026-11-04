# Zendure Control

ESP32 firmware that adjusts a local Zendure battery's charge or discharge power using net-consumption measurements from an Enphase gateway.

The firmware runs on an ESP32 with the Arduino framework. It polls both devices over the local network and sends a battery setpoint approximately once per second. It also obtains an Enphase Enlighten token to read the gateway's consumption endpoint.

## How it works

The ESP32 reads consumption data from the Enphase gateway and battery data from the Zendure device, then sends a power request to the battery. The control strategy is subject to change.

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

`TEST_CHARGE` in `src/main.cpp` is a compile-time switch and is `false` by default. When enabled, it bypasses normal regulation and sends charge requests that step from 50 W to 300 W in 50 W increments, approximately every five seconds. Use this only when deliberately testing the battery in a controlled setup, and restore it to `false` for normal operation.

## Security and operational notes

- The Enlighten login/token requests validate TLS using `ENPHASE_ROOT_CA`.
- The Enphase gateway HTTPS client currently calls `setInsecure()`, so it does **not** validate the gateway's TLS certificate. This should be addressed before using the firmware on an untrusted network.
- Zendure local API requests use plain HTTP. Keep the device and ESP32 on a trusted, access-controlled local network.
- This firmware can actively control battery power. Verify IP addresses, power limits, SOC thresholds, device compatibility, and the behavior of your installation before enabling unattended operation.
- If a measurement is stale or unavailable, the calculated setpoint is zero. Check the serial output if the battery repeatedly receives zero-power requests.

## Project layout

| Path | Contents |
| --- | --- |
| `src/main.cpp` | Wi-Fi setup, Enphase authentication, polling, control calculation, and main loop |
| `include/zendure.h` | Zendure local API reads and setpoint writes |
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