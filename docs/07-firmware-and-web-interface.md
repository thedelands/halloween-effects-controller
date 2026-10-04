# 7. Firmware and Web Interface

## Firmware services

- Sensor manager
- Blower PWM manager
- Button input/debounce
- Operating-mode state machine
- Wi-Fi station and fallback access point
- HTTP API
- Static dashboard
- OLED status renderer
- OTA update handler
- Preferences storage

## REST endpoints

| Method | Path | Purpose |
|---|---|---|
| GET | `/api/status` | Current readings and state |
| POST | `/api/blower` | Set manual speed |
| POST | `/api/mode` | Set `manual`, `auto`, or `test` |
| POST | `/api/stop` | Immediate blower stop |
| POST | `/api/test` | Timed blower test |
| GET | `/healthz` | Basic liveness |

## Example status

```json
{
  "mode": "manual",
  "blowerPercent": 35,
  "ambientTempF": 48.2,
  "humidity": 77.1,
  "pressureHpa": 1009.4,
  "coolerTempF": 36.0,
  "outletTempF": 38.1,
  "wifiRssi": -58,
  "uptimeSeconds": 812
}
```

## Automatic mode, Version 1

The initial rule set is intentionally simple:

1. If no cooler sensor is available, stop and report a fault.
2. If cooler temperature is above the configured warm threshold, run only the configured purge speed.
3. Otherwise calculate a base blower command from the temperature difference between ambient and outlet.
4. Clamp command between minimum and maximum limits.
5. Apply a startup boost before low-speed operation.

Do not treat this as safety-critical feedback control.

## Web interface

The dashboard in `firmware/data/index.html` is mobile-first and provides:

- live sensor cards
- blower slider
- mode selection
- start/stop
- timed test
- connection state
- automatic polling

The firmware scaffold also embeds a minimal fallback page. Later, the filesystem-hosted dashboard can be added using LittleFS.

## Security

Version 1 is intended for a trusted private LAN.

Before exposing it to a larger network:

- set a unique OTA password
- add HTTP authentication
- do not port-forward the device
- isolate it on an IoT VLAN if available
- disable the setup AP after provisioning
