# 🚇 Smart Tunnel Light Controller

> ESP32-based tunnel lighting system with Wi-Fi web control, auto-off timer, and sequential lighting mode.

**West Yangon Technological University — Department of Electronic Engineering**
*II B.E. EC ·
---

## 📌 Overview

Traditional tunnel lighting wastes energy through manual or fixed-timer operation. This project replaces that with an **IoT-based smart controller** built on the ESP32 microcontroller. Any smartphone or laptop can open a browser and control the tunnel LEDs in real time — no app needed.

### Key Features

| Feature | Description |
|---|---|
| Individual control | Turn each LED on/off separately |
| Group control | All ON / All OFF with one tap |
| Sequential mode | LEDs cycle one by one (simulates vehicle passage) |
| Auto-off timer | Each LED turns off automatically after a set delay |
| Adjustable timing | Control sequential delay & auto-off from the web UI |
| Zero sensors needed | Fully web-command driven |

---

## 🛠 Hardware

### Components

| # | Component | Specification |
|---|---|---|
| 1 | ESP32 Dev Board | DOIT ESP32 DEVKIT V1 |
| 2 | LEDs | 0.5 mm, red/green/blue |
| 3 | Resistors | 220 Ω × 3 |
| 4 | Control device | Any smartphone or PC |

### Wiring

```
ESP32 GPIO2  ──► 220Ω ──► LED1 (anode) ──► GND
ESP32 GPIO4  ──► 220Ω ──► LED2 (anode) ──► GND
ESP32 GPIO5  ──► 220Ω ──► LED3 (anode) ──► GND
```

> Each LED operates at ~3 V. The 220 Ω resistor limits current to a safe ~14 mA.

---

## 💻 Software Setup

### Requirements

- [Arduino IDE 2.x](https://www.arduino.cc/en/software)
- ESP32 board package by Espressif

### Install ESP32 Board Package

1. Open Arduino IDE → **File → Preferences**
2. Paste into *Additional Boards Manager URLs*:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager** → search `esp32` → Install

### Flash the Firmware

1. Clone this repo:
   ```bash
   git clone https://github.com/YOUR_USERNAME/smart-tunnel-light-controller.git
   ```
2. Open `src/tunnel_light_controller.ino` in Arduino IDE
3. Select board: **Tools → Board → DOIT ESP32 DEVKIT V1**
4. Select the correct COM port
5. Click **Upload**

---

## 🌐 Usage

1. After flashing, the ESP32 creates a Wi-Fi access point:
   - **SSID:** `TunnelLight_AP`
   - **Password:** `tunnel123`

2. Connect your phone or PC to that network.

3. Open a browser and go to:
   ```
   http://192.168.4.1
   ```

4. Use the web interface to control the lights.

### Web API Endpoints

| Endpoint | Action |
|---|---|
| `GET /` | Load control page |
| `GET /on` | All LEDs ON |
| `GET /off` | All LEDs OFF |
| `GET /toggle?led=1` | Toggle LED 1 (1, 2, or 3) |
| `GET /seq` | Start sequential mode |
| `GET /seqstop` | Stop sequential mode |
| `GET /set?timer=5000` | Set auto-off to 5000 ms |
| `GET /set?seqDelay=800` | Set sequential delay to 800 ms |
| `GET /status` | JSON status of all LEDs |

---

## ⚙️ Configuration

Edit these constants at the top of the `.ino` file:

```cpp
const char* AP_SSID     = "TunnelLight_AP";  // Wi-Fi network name
const char* AP_PASSWORD = "tunnel123";        // Wi-Fi password (min 8 chars)

const int LED_PINS[3] = {2, 4, 5};           // GPIO pins for LED1, LED2, LED3
```

Default timer values (changeable live from the web UI):

```cpp
unsigned long autoOffMs  = 5000;   // Auto-off: 5 seconds
unsigned long seqDelayMs = 800;    // Sequential delay: 0.8 seconds
```

---

## 📊 System Flow

```
Power ON
   │
   ▼
Initialize ESP32 + GPIO + Wi-Fi AP + Web Server
   │
   ▼
Wait for Client Connection
   │
   ▼
Receive URL Command ──────────────────────────────────┐
   │                                                  │
   ├─ /on       → All LEDs ON                         │
   ├─ /off      → All LEDs OFF                        │
   ├─ /toggle   → Toggle individual LED               │
   ├─ /seq      → Start sequential mode               │
   ├─ /set      → Update timer / delay settings       │
   └─ /status   → Return JSON state                   │
                                                      │
Loop: Check auto-off timers + advance sequential ─────┘
```

---

## ✅ Advantages

- No app required — works in any browser
- Low cost (ESP32 + basic components)
- Real-time response to web commands
- Sequential mode simulates real tunnel lighting
- Energy-saving auto-off timer

## ⚠️ Limitations

- Wi-Fi range limited to ~30–50 m (local AP only)
- No authentication on the web interface
- No physical sensors (light/motion)
- Sequential delay must be changed via web UI (not hard-coded per segment)

---

## 🔮 Future Improvements

- [ ] Add light-dependent resistor (LDR) for ambient brightness sensing
- [ ] Integrate PIR motion sensor for vehicle detection
- [ ] Connect to cloud (MQTT / Firebase) for remote internet access
- [ ] Add login/authentication to the web interface
- [ ] PWM dimming control per LED
- [ ] Mobile-responsive progressive web app (PWA)

---

## 📁 Project Structure

```
smart-tunnel-light-controller/
├── src/
│   └── tunnel_light_controller.ino   # Main Arduino firmware
├── circuit/
│   └── wiring_diagram.md             # Pin connections reference
├── images/
│   └── (prototype photos)
└── README.md
```
**Institution:** West Yangon Technological University, Department of Electronic Engineering

---

## 📄 License

This project is released for educational purposes.
