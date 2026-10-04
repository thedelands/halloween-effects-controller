# 1. Safety and Design Decisions

## Electrical boundaries

The system has three electrical zones:

1. **120 V AC:** GFCI-protected source and the AC input of the approved 12 V supply.
2. **12 V DC power:** WAGO distribution, branch fuses, blower, MOSFET power path, and LM2596 input.
3. **3.3 V control:** Arduino Nano ESP32 GPIO, sensors, OLED, and buttons.

The tested baseline keeps the ALITOVE AC/DC adapter outside the controller enclosure and routes only its 12 V output through a cable gland. Keep that adapter dry and ventilated.

A future cabinet-rated DIN-rail AC/DC supply may be installed inside the weatherproof enclosure only in a covered mains compartment with suitable cable entry, disconnect/overcurrent protection, guarding, spacing, and any required protective-earth bonding. This is a separate mains-wiring upgrade, not part of the low-voltage WAGO conversion. Have that installation performed or inspected by a qualified electrician.

## Powering the Nano ESP32

Set the LM2596 output to **7.0 V DC** and connect it to:

- LM2596 OUT+ → Nano ESP32 VIN
- LM2596 OUT− → Nano ESP32 GND

The official operating range for VIN is 6–21 V. Do not connect the 12 V supply directly to a 3.3 V or VBUS pin.

Adjust and verify the LM2596 output with a multimeter **before** connecting the Nano.

## Fusing

Recommended branch protection:

- Blower branch: 5 A ATC/ATO fuse
- Controller branch: 2 A or 3 A ATC/ATO fuse

Place each fuse close to the 12 V distribution point.

The permanent low-voltage layout is:

```text
12 V supply +V -- +12V WAGO -- branch fuse -- load positive
12 V supply -V -- bridged 0V WAGOs --------- load return
```

Do not fuse the shared 0 V returns. Do not put more than one conductor in a WAGO lever position.

## Motor switching

The blower must not be powered from an ESP32 pin. The ESP32 supplies only a PWM logic signal to the MOSFET module.

Requirements for the selected MOSFET module:

- Accepts 3.3 V logic-level control
- Supports PWM
- Supports a 12 V inductive motor load
- Continuous current comfortably above 2.5 A
- Common ground between MOSFET input and ESP32

Because generic board layouts differ, identify:

- Power input positive/negative
- Load output positive/negative
- Signal/PWM input
- Logic ground

Do not rely on a wiring diagram from a visually similar board.

## Outdoor installation

- Use an IP65 or better enclosure.
- Use downward-facing cable glands where practical.
- Add drip loops.
- Mount the BME280 in a ventilated radiation shield, not sealed inside the enclosure.
- Keep the DS18B20 cable joints outside standing water.
- Mount the controller above grade.
- Do not place the AC adapter inside the wet cooler.
- Do not leave the assembly outdoors permanently.

## Fog-fluid note

The controller package does not include a fog-machine heater or fluid-control design. Use commercial fog fluid during initial chiller testing. Homemade vegetable-glycerin mixtures may void a fog-machine warranty and can increase residue or clogging.
