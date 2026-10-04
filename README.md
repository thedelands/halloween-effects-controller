# Halloween Effects Controller — Fog Chiller Module

A reusable ESP32-based controller for a 12 V low-fog chiller blower, environmental monitoring, local OLED status, and a phone-friendly web interface.

## Current scope

Version 1 controls and monitors:

- SEAFLO 3-inch, 12 V inline blower through MOSFET A, with MOSFETs B and C reserved for future loads
- Two or more DS18B20 waterproof temperature probes
- One BME280 ambient temperature/humidity/pressure sensor
- One SSD1306 128×64 I²C OLED
- Three momentary pushbuttons
- Wi-Fi web dashboard
- Manual and automatic blower modes
- OTA firmware updates
- Reserved outputs for a future isolated fog-machine trigger and motion sensor

The fog machine and cooler are intentionally deferred.

## Start here

1. Read `docs/01-safety-and-design-decisions.md`.
2. Review `docs/02-system-architecture.md`.
3. Follow `docs/03-wiring-and-pin-map.md`.
4. Bench-test using `docs/06-bench-build-and-test-plan.md`.
5. Build and upload the firmware from `firmware/`.

## Critical power correction

Power the Arduino Nano ESP32 from the LM2596 through **VIN at approximately 7.0 V**, not by injecting 5 V into a USB/VBUS pin. The official Nano ESP32 documentation specifies a **6–21 V VIN input range**.

The permanent low-voltage build uses three physical WAGO blocks containing separate groups for +12 V, common 0 V, 3.3 V, DS18B20 data, SDA, and SCL. Every positive 12 V branch is fused after the +12 V WAGO and before its device. The as-built three-MOSFET layout uses eight 221-415 and three 221-413 connectors; see `docs/03-wiring-and-pin-map.md` for the slot-by-slot map.

## Repository layout

```text
halloween-effects-controller/
├── .github/
├── .gitignore
├── .gitattributes
├── LICENSE
├── README.md
├── CHANGELOG.md
├── CONTRIBUTING.md
├── CODE_OF_CONDUCT.md
├── BOM.csv
├── docs/
│   ├── 01-safety-and-design-decisions.md
│   ├── 02-system-architecture.md
│   ├── 03-wiring-and-pin-map.md
│   ├── 04-enclosure-layout.md
│   ├── 05-cooler-module-plan.md
│   ├── 06-bench-build-and-test-plan.md
│   ├── 07-firmware-and-web-interface.md
│   └── 08-roadmap.md
├── firmware/
│   ├── platformio.ini
│   ├── include/fog_controller_config.example.h
│   ├── src/main.cpp
│   └── data/index.html
├── hardware/
│   ├── architecture.svg
│   ├── enclosure-mounting-grid.svg
│   ├── wago-block-map.svg
│   └── wiring-overview.svg
├── cad/
├── assets/
├── examples/
├── tools/
└── manifest.json
```

## Important limitation

The exact screw-terminal order and protection components vary among generic MOSFET modules and LM2596 boards. Confirm terminal labels and polarity printed on the actual boards before applying power.
