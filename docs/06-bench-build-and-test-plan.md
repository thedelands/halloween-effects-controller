# 6. Bench Build and Test Plan

Do not connect every component at once.

## Stage 1 — Power conversion

1. Leave the Nano disconnected.
2. Install the separate +12 V and 0 V WAGO distribution connectors.
3. With all branch fuses removed, verify approximately 12 V between the distribution connectors.
4. Confirm there is no direct short between +12 V and 0 V with power disconnected.
5. Install only the 2–3 A controller fuse and connect the LM2596 input.
6. Measure input polarity.
7. Adjust output to **7.0 V**.
8. Power-cycle and confirm the setting remains stable.
9. Connect OUT+ to VIN and OUT−/Nano GND according to the 0 V map.
10. Confirm Nano boots.

Pass criteria:

- 12 V input within adapter tolerance
- LM2596 output 6.8–7.2 V
- No excessive heat
- Nano power LED stable

## Stage 2 — OLED and BME280

1. Connect only I²C devices.
2. Upload the firmware.
3. Check serial output for OLED address and BME280 address.
4. Compare ambient readings against a known thermometer/hygrometer.

Pass criteria:

- OLED initializes at 0x3C
- BME280 found at 0x76 or 0x77
- Readings update without I²C errors

## Stage 3 — DS18B20 probes

1. Connect one probe with the 4.7 kΩ pull-up.
2. Confirm its unique address and temperature.
3. Add the second probe.
4. Label probes physically with their discovered addresses.
5. Test each in room air and chilled water without submerging cable splices.

Pass criteria:

- No `-127 °C` disconnected reading
- No persistent `85 °C` startup reading
- Both addresses remain stable

## Stage 4 — Buttons

Verify:

- Start/Stop toggles blower command
- Mode cycles Manual → Auto → Test
- Test button runs a timed low-speed test

## Stage 5 — MOSFET without blower

1. Disconnect blower.
2. Confirm MOSFET signal input sees 0–3.3 V logic.
3. Confirm wiring against labels on the actual module.
4. Use a small 12 V test lamp if available.

## Stage 6 — Blower

1. Install 5 A fuse.
2. Secure blower so torque cannot move it.
3. Start at 20%.
4. Increase in 10% steps.
5. Record:
   - reliable startup threshold
   - noise
   - MOSFET temperature
   - adapter temperature
   - measured current if a clamp meter is available
6. Run 30 minutes at the intended maximum.

Initial software limits:

- Minimum running speed: 25%
- Maximum speed: 70%
- Startup boost: 60% for 750 ms, then requested speed

Tune these values from actual testing.

## Stage 7 — Network

Test:

- Wi-Fi connection
- setup access point fallback
- `fogbox.local`
- dashboard controls
- OTA update
- recovery after router outage

## Failure response

Immediately disconnect power if:

- wiring insulation softens
- barrel connector or adapter becomes abnormally hot
- fuse holder discolors
- blower stalls
- MOSFET module smells hot
- buck output exceeds 7.2 V
