# ESP32 Cat Feeder (Cat Mate 500 + Home Assistant)

This project started for one simple reason: I needed to feed my cat properly when I was away. she only eats very small meals multiple times a day. 

The **Cat Mate C500** is great feeder, but it is built around a short timer cycle if only 24 hours. I wanted something I could control remotely and schedule flexibly with **Home Assistant**, including windows longer than 24 hours.

With two C500 units and ice packs, I can keep 8 small meals fresh for around **48 hours**. That gives me a much better safety margin for overnight trips and weekends for work. I prefer being able to watch on camera and judge when she needs feeding rather than 
merely a timer.


# ESP32-C3 Cat Feeder

A hacked C500 cat feeder controlled via a web interface and Home Assistant, built around an ESP32-C3 board with built-in OLED.

---

## Components

| Component | Value / Part | Purpose |
|---|---|---|
| ESP32-C3 with OLED | Diymore ESP32-C3 | Main controller, web server, display |
| IR Optical Sensor | ITR9608 | Detects bowl position, has one already but this is more reliable and is 3.3v compatable.. ok i might have blown up the originals... |
| NPN Transistor | 2N2222A | Motor switching |
| Flyback Diode | 1N4007 | Motor spike protection |
| Resistor | 1kΩ  | Transistor base current limiting |
| Resistor | 200Ω | IR LED current limiting |
| USB-C Breakout Board | With CC1/CC2 pins | Power input for mounting the power elsewhere |

---

## Wiring

### Power
```
USB 5V              →  V5 pin on ESP32
USB GND             →  GND pin on ESP32
CC1 on breakout     →  5.1kΩ  →  GND  (if not already on breakout board)
CC2 on breakout     →  5.1kΩ  →  GND  (if not already on breakout board)
```

### Motor Circuit
```
V3 (3.3V)           →  Motor +
Motor -             →  2N2222A Collector (middle leg, flat face forward)
2N2222A Emitter     →  GND              (left leg, flat face forward)
GPIO 3              →  1kΩ  →  2N2222A Base  (right leg, flat face forward)

Flyback diode across motor terminals:
  Stripe (cathode)  →  Motor + (3.3V side)
  No stripe (anode) →  Motor - (Collector side)
```

### IR Sensor (ITR9608)
```
Pin 1 (Anode)       →  200Ω  →  3.3V   (emitter/LED side)
Pin 2 (Cathode)     →  GND              (emitter/LED side)
Pin 3 (Collector)   →  GPIO 4           (receiver/output side)
Pin 4 (Emitter)     →  GND              (receiver/output side)
```

> Note: GPIO 4 uses INPUT_PULLUP in code — no external pull up resistor needed.

### OLED Display
```
Internal to board — no wiring needed
SDA  →  GPIO 5 (internal)
SCL  →  GPIO 6 (internal)
```

### Boot Button
```
Internal to board — no wiring needed
GPIO 9 (internal)
```

---

## GPIO Summary

| GPIO | Function |
|---|---|
| GPIO 3 | Motor control (via 2N2222A) |
| GPIO 4 | IR sensor signal |
| GPIO 5 | OLED SDA (internal) |
| GPIO 6 | OLED SCL (internal) |
| GPIO 9 | Boot button (internal) |

---

## Libraries Required

Install via Arduino IDE Library Manager:

- **U8g2** by oliver
- **WiFiManager** by tzapu

---

## Usage

### First Boot
1. Power on the device
2. Connect to the **CATFEEDER###** WiFi network (no password)
3. Open browser to **192.168.4.1**
4. Enter your WiFi details and select DHCP or Static IP
5. Device restarts and connects

### Web Interface
Navigate to the device IP address in any browser to access the feed button.

### Manual Feed
Press the boot button once to trigger a feed.

### Reset WiFi
Hold the boot button for 10 seconds — device clears WiFi settings and restarts into AP mode.

### Home Assistant
Add to `configuration.yaml`:
```yaml
rest_command:
  cat_feeder_1:
    url: "http://<device-ip>/feed"
    method: GET
```

Then call `rest_command.cat_feeder_1` from automations or the dashboard.

---

## Notes

- Motor runs on 3.3V — original feeder ran on 2-3x AA batteries so 3.3V is fine
- IR sensor requires 200Ω current limiting resistor on LED side — without it the LED will burn out
- WiFi TX power is reduced to `WIFI_POWER_8_5dBm` to fix cold boot connection issues on this board
- USB-C breakout board must have CC1 and CC2 pulled to GND via 5.1kΩ resistors to negotiate power from charger


## Libraries used

- `WiFi.h`
- `WebServer.h`
- `WiFiManager.h`
- `Preferences.h`
- `Wire.h`
- `U8g2lib.h`

Install these in Arduino IDE before compiling.

## Upload and run

1. Open `cat_feeder/cat_feeder.ino` in Arduino IDE.
2. Install required libraries.
3. Select your ESP32 board + COM port.
4. Upload firmware.
5. On first boot, connect to AP shown on OLED (`CATFEEDERxxx`) and set Wi-Fi.
6. Open the ESP32 IP address in browser or call `/feed` from Home Assistant.


## 3d Prints

Included 3 STL's for mounting the board in the space the existing LCD goes and for mounting external USB C breakout board with a little super glue! 

Enjoy! Spend time with your cat and don't let the robots do all the feeding!
