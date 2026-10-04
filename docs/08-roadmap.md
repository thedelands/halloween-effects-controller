# 8. Development Roadmap

## Milestone 1 — Bench controller

- Power conversion verified
- OLED operational
- BME280 operational
- Two DS18B20 probes discovered and labeled
- Buttons operational
- Web dashboard reachable

## Milestone 2 — Blower control

- MOSFET terminal mapping verified
- Startup threshold characterized
- PWM range calibrated
- 30-minute thermal test passed
- Emergency stop works from button and web UI

## Milestone 3 — Enclosure

- Mounting plate populated
- Cable glands installed
- Sensor connectors labeled
- BME280 radiation shield installed
- Rain/drip test completed with power removed

## Milestone 4 — Cooler integration

- Cooler selected
- 3-inch inlet/outlet cut plan finalized
- Baffles installed
- Condensate drainage verified
- Temperature-drop tests completed using room air

## Milestone 5 — Fog-machine integration

Deferred until a machine is selected.

Requirements:

- characterize remote connector voltage and behavior
- use galvanic isolation
- preserve manufacturer interlocks and heater controls
- never switch internal mains heater wiring from the ESP32

## Milestone 6 — Effects platform

Potential additions:

- PIR or beam sensor
- DMX output
- WS2812 landscape lighting
- audio trigger
- MQTT/Home Assistant
- scene scheduler
- multiple blower/fog zones
- GitHub Actions firmware build
- signed release artifacts
