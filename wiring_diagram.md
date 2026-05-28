# Wiring Diagram — Smart Tunnel Light Controller

## Pin Connections

```
┌─────────────────────────────────────────────────┐
│                 ESP32 Dev Kit V1                │
│                                                 │
│  GPIO2  ──────┬── 220Ω ──── LED1(+) ──── GND   │
│  GPIO4  ──────┼── 220Ω ──── LED2(+) ──── GND   │
│  GPIO5  ──────┴── 220Ω ──── LED3(+) ──── GND   │
│                                                 │
│  3V3 / GND for power (USB or external 5V)       │
└─────────────────────────────────────────────────┘
```

## Component Details

| Component | Value | Purpose |
|---|---|---|
| Resistor R1, R2, R3 | 220 Ω | Current limiting for LEDs |
| LED1, LED2, LED3 | 0.5 mm, ~2 V forward voltage | Tunnel light simulation |
| ESP32 GPIO output | 3.3 V logic HIGH | Drive LED via resistor |

## Current Calculation

```
I = (Vcc - Vf) / R
I = (3.3V - 2.0V) / 220Ω
I ≈ 5.9 mA  (safe; ESP32 GPIO max is 40 mA per pin)
```

## Notes

- All LEDs share a common GND with the ESP32.
- No external power supply needed for 3 LEDs at this current.
- For real tunnel lights, use MOSFETs or relays driven by these GPIO pins.
