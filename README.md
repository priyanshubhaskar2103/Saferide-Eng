# 🚗 SafeRide Sentinel

> **An embedded vehicle safety system for real-time alcohol detection, fire hazard monitoring, and emergency response.**

Built for the **Innovate Bharat Hackathon 2026** — School Student Innovation Track (SCHI103)
Hosted by **Sharda University**

---

## 👥 Team — SafeRide Engineers

| Name | Role |
|---|---|
| Priyanshu Bhaskar | Hardware Integration & ESP32 Programming |
| Somya Agarwal | Sensor Logic & Threshold Calibration |
| Preeti Patel | GPS & Communication (Twilio/NEO-6M) |
| Raj Sisoudia | Circuit Design & System Testing |

---

## 🧩 What It Does

SafeRide Sentinel is a smart embedded system fitted into a vehicle that continuously monitors for two critical threats:

**Alcohol Detection** — The MQ-3 sensor reads ethanol concentration near the driver. If the reading crosses the calibrated threshold, the system initiates a gradual speed reduction, locks the vehicle to 1st gear (max ~25 km/h), and dispatches an SMS alert with live GPS coordinates to emergency contacts via Twilio.

**Fire Detection** — A flame sensor monitors for infrared signatures of fire or abnormal heat inside the vehicle. On detection, the relay cuts the ignition immediately, the buzzer fires, and an emergency SMS is sent.

**SOS / Hazard Button** — A dedicated hardware button allows the driver to manually trigger an emergency alert at any time, sending their GPS location to contacts even if no sensor has triggered.

---

## ⚙️ Hardware Components

| Component | Purpose | Pin |
|---|---|---|
| ESP32 (SoC) | Central microcontroller | — |
| MQ-3 Alcohol Sensor | Detects ethanol in cabin air | GPIO 34 (ADC1) |
| Flame Sensor | Detects fire / IR heat signature | GPIO 25 |
| NEO-6M GPS Module | Real-time location tracking | GPIO 16 (RX), 17 (TX) |
| Relay Module | Controls vehicle ignition / motor | GPIO 23 |
| Buzzer | In-vehicle audio alert | GPIO 22 |
| LED Indicator | Visual system status | GPIO 21 |
| Hazard / SOS Button | Manual emergency trigger | GPIO 18 |
| 3.7V LiPo Battery | Power supply | — |

> ⚠️ **ADC Note:** MQ-3 must be on an ADC1 pin (GPIO 32–39). ADC2 pins (GPIO 0, 2, 4, 12–15, 25–27) are disabled when WiFi is active on ESP32 and will cause silent sensor failures.

---

## 🔌 Power Rail

| Rail | Voltage | Powers |
|---|---|---|
| LiPo Battery | 3.7V | System input |
| MQ-3 VCC | 5V | Alcohol sensor heater |
| Flame Sensor VCC | 3.3V | Flame sensor logic |
| ESP32 VCC | 3.3V | Microcontroller |

---

## 🛠️ Software Stack

- **Language:** C / C++ (Arduino framework on ESP32)
- **Libraries:** TinyGPS++, WiFi, HTTPClient, FreeRTOS (built-in ESP32)
- **Communication:** Twilio REST API (SMS + Call alerts)
- **Architecture:** FreeRTOS multi-task — sensors on Core 0, GPS + Twilio on Core 1

---

## 🏗️ System Architecture

```
┌─────────────────────────────────┐
│           ESP32 (Dual Core)     │
│                                 │
│  CORE 0 (Sensors & Control)     │
│  ├── MQ-3 read (100ms loop)     │
│  ├── Flame sensor poll          │
│  ├── Hazard button check        │
│  └── Relay control              │
│                                 │
│  CORE 1 (Comms)                 │
│  ├── NEO-6M GPS parse           │
│  └── Twilio SMS / Call          │
│                                 │
│  Shared volatile flags:         │
│  alcoholDetected, fireDetected, │
│  sosPressed, gpsLat, gpsLon     │
└─────────────────────────────────┘
```

---

## 🔄 System Flow

```
Power ON
   │
   ▼
ESP32 Boot → Sensor warm-up (MQ-3 ~60s preheat)
   │
   ▼
Continuous monitoring loop
   │
   ├──[Alcohol > 1.8V threshold]──▶ Gradual speed reduction (10s)
   │                                 → Lock to 1st gear (max 25 km/h)
   │                                 → SMS + GPS via Twilio
   │                                 → Buzzer alert
   │                                 → Re-test required to unlock
   │
   ├──[Flame sensor HIGH]─────────▶ Relay cuts ignition immediately
   │                                 → SMS + GPS via Twilio
   │                                 → Buzzer alarm
   │
   └──[SOS button pressed]────────▶ SMS + GPS via Twilio
                                     → Buzzer pulse
```

---

## 🚦 Alcohol Detection — Speed Limiting Logic

Unlike a simple ignition cutoff, SafeRide Sentinel uses a **graduated response** that keeps the driver in control while preventing dangerous speeds:

1. Alcohol detected → 10 second countdown begins
2. Vehicle speed reduces progressively: 4th → 3rd → 2nd → 1st gear equivalent
3. After 10 seconds: speed locked at max 25 km/h (1st gear only)
4. Lock remains for **10 minutes**
5. Re-test available after 10 minutes — pass clears the lock, fail keeps it active

This approach avoids sudden stops on highways while still preventing dangerous drunk driving.

---

## 📡 False Positive Mitigation

The MQ-3 is an ambient air sensor, not a breath-specific sensor. To reduce false positives from passengers, spills, or clothing:

- Sensor is positioned close to the **driver-side steering column**, not cabin centre
- Detection only activates in the **ignition window** (first 3 seconds after key turn), not continuously
- System uses **delta-based triggering** — reads a baseline on door unlock, triggers only if the spike significantly exceeds that baseline
- Threshold is **firmware-adjustable** and tuned during real-world calibration runs

---

## 🚀 Setup & Flashing

**Prerequisites:**
- Arduino IDE or PlatformIO
- ESP32 board package installed
- Libraries: `TinyGPS++`, `WiFi` (built-in), `HTTPClient` (built-in)

**Steps:**

1. Clone the repository
2. Open `include/config.h` and fill in:
   ```cpp
   #define WIFI_SSID      "your_wifi"
   #define WIFI_PASSWORD  "your_password"
   #define TWILIO_SID     "your_sid"
   #define TWILIO_TOKEN   "your_token"
   #define TWILIO_FROM    "+1XXXXXXXXXX"
   #define ALERT_TO       "+91XXXXXXXXXX"
   ```
3. Select board: **ESP32 Dev Module**
4. Flash and open Serial Monitor at **115200 baud**
5. Wait ~60 seconds for MQ-3 sensor warm-up before testing

---

## 🔮 Future Scope

- Camera-based driver drowsiness detection
- OBD-II integration for actual gear and speed data
- Mobile app dashboard for fleet managers
- Cloud logging of all alert events with timestamps
- GSM fallback (SIM7600) for areas without WiFi

---

## 📄 License

This project was created for educational and hackathon purposes.
© 2026 SafeRide Engineers — Innovate Bharat Hackathon, Sharda University.
