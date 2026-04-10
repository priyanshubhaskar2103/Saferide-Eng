

| SAFERIDE SENTINEL Vehicle Safety System — Technical Documentation Using Fire & Alcohol Detection  •  ESP32  •  Twilio  •  NEO-6M GPS |
| :---: |

| Team | SafeRide Engineers |
| :---- | :---- |
| **Team ID** | SCHI103 |
| **Members** | Priyanshu Bhaskar  •  Somya Agarwal  •  Preeti Patel  •  Raj Sisoudia |
| **Event** | Innovate Bharat Hackathon 2026 — Sharda University |
| **Track** | School Student Innovation (SCHI103) |
| **Version** | 1.0 \- April 2026 |

# **1\. Executive Summary**

SafeRide Sentinel is an embedded vehicle safety system designed to prevent road accidents caused by drunk driving and undetected vehicle fires. The system integrates multiple sensors, a microcontroller, GPS tracking, and cloud-based SMS alerting into a unified, always-on platform that responds automatically when a threat is detected — without requiring any action from the driver.

The core philosophy is prevention over punishment. Rather than simply cutting the engine and stranding a driver, SafeRide Sentinel implements a graduated response: gradual speed reduction, gear locking at a safe minimum, and re-testing before restoring full control. This approach acknowledges real-world edge cases while still enforcing safety.

| Detection Time | Alert Delay | Min Speed Lock | Lock Duration |
| :---: | :---: | :---: | :---: |
| **\< 100 ms** | **2 \- 5 sec** | **25 km/h** | **10 minutes** |

# **2\. Problem Statement**

Road accidents remain one of the leading causes of preventable deaths in India and globally. Two specific hazards that this project addresses are drunk driving and vehicle fires — both of which are often detectable early enough to prevent fatality, but lack automated intervention systems in most vehicles.

## **2.1 Drunk Driving**

According to the National Crime Records Bureau (NCRB), drunk driving accounts for a significant proportion of fatal road accidents in India annually. The problem is compounded by: 

* Slow driver reaction times after alcohol consumption

* Poor judgment and risk assessment while intoxicated

* Lack of any onboard mechanism to detect or prevent drunk driving in standard vehicles

* Delayed emergency response due to no automatic location sharing

## **2.2 Vehicle Fires**

* Vehicle fires often start small and go undetected until they are uncontrollable

* Engine compartment fires from fuel leaks or electrical faults can spread in under 60 seconds

* Drivers are frequently unaware until smoke enters the cabin, by which time it may be too late to safely stop

* No standard consumer vehicle includes an automatic fire-detection and response system

## **2.3 Gap in Existing Solutions**

Existing solutions such as breathalyzer checkpoints are reactive and intermittent. Commercial ignition interlock devices exist but are expensive, require installation by specialists, and are only mandated for repeat offenders in most jurisdictions. There is no affordable, scalable, embedded solution for general fleet use.

| Problem in One Line Vehicles lack an affordable, real-time, automated system that detects driver impairment and fire hazards, takes immediate corrective action, and notifies emergency contacts with location data — all without driver intervention. |
| :---- |

# **3\. Solution Overview**

SafeRide Sentinel is a smart embedded system that mounts inside a vehicle and continuously monitors for two critical threats: alcohol presence near the driver, and fire or abnormal heat within the vehicle. On detection, it executes a pre-programmed response sequence automatically.

## **3.1 Alcohol Detection Response**

1. MQ-3 sensor detects ethanol concentration above the calibrated threshold (1.8V on a 5V rail)

2. 10-second countdown begins — displayed to driver via buzzer pattern and serial output

3. Vehicle speed is gradually reduced through relay-controlled motor limiting: 4th gear equivalent down to 1st

4. After 10 seconds: speed locked at maximum 25 km/h (1st gear only)

5. Twilio SMS dispatched to emergency contacts with live GPS coordinates

6. Lock remains active for 10 minutes minimum

7. Re-test available after 10 minutes — passing clears the lock, failing extends it

## **3.2 Fire Detection Response**

8. Flame sensor detects infrared radiation signature of fire or abnormal heat

9. Relay module immediately cuts vehicle ignition

10. Buzzer alarm activates

11. Emergency SMS sent via Twilio with GPS coordinates

## **3.3 SOS / Hazard Button**

12. Driver presses dedicated hardware button at any time

13. System immediately sends SMS alert with current GPS location to emergency contacts

14. Buzzer pulses to confirm transmission

15. Does not cut the engine — driver remains in control

# **4\. Hardware Components**

## **4.1 Component Specifications**

| Component | GPIO Pin | Voltage | Function |
| ----- | ----- | ----- | ----- |
| ESP32 Dev Module | — | 3.3V VCC | Central microcontroller, dual-core, WiFi built-in |
| MQ-3 Alcohol Sensor | GPIO 34 (ADC1) | 5V VCC | Detects ethanol concentration in cabin air via analog output |
| Flame Sensor Module | GPIO 25 | 3.3V VCC | Detects infrared radiation from fire or abnormal heat |
| NEO-6M GPS Module | GPIO 16 (RX), 17 (TX) | 3.3V VCC | Real-time latitude, longitude, altitude, speed via NMEA |
| Relay Module | GPIO 23 (IN) | 5V coil | Controls vehicle ignition circuit and motor power |
| Buzzer | GPIO 22 | 3.3V | Audio alert for alcohol, fire, and SOS events |
| LED Indicator | GPIO 21 | 3.3V | Visual system status — green normal, red alert |
| Hazard / SOS Button | GPIO 18 | 3.3V (pull-up) | Manual emergency trigger for driver |
| 3.7V LiPo Battery | — | 3.7V nominal | Primary power source for the embedded system |

| Critical ADC Warning The MQ-3 sensor MUST be connected to an ADC1 pin (GPIO 32–39). ADC2 pins (GPIO 0, 2, 4, 12–15, 25–27) are hardware-shared with the ESP32 WiFi radio and become non-functional when WiFi is active. Using an ADC2 pin for MQ-3 will cause the alcohol sensor to silently fail whenever Twilio sends an SMS. |
| :---- |

## **4.2 Power Architecture**

| Power Rail | Voltage | Supplies |
| ----- | ----- | ----- |
| LiPo Battery (input) | 3.7V nominal | System input power source |
| MQ-3 Heater Circuit | 5V (via boost converter) | Alcohol sensor heating element — required for accurate readings |
| Flame Sensor Logic | 3.3V | Flame sensor VCC and digital output logic level |
| ESP32 Core | 3.3V (onboard regulator) | Microcontroller, GPIO peripherals, WiFi radio |
| Relay Coil | 5V | Relay activation — requires 5V coil even if trigger pin is 3.3V |

# **5\. Software Architecture**

## **5.1 Technology Stack**

| Layer | Technology |
| ----- | ----- |
| Language | C / C++ (Arduino framework on ESP32) |
| RTOS | FreeRTOS (built into ESP32 Arduino core) |
| GPS Library | TinyGPS++ for NMEA sentence parsing |
| Communication | Twilio REST API over HTTPS for SMS and voice alerts |
| Networking | ESP32 built-in WiFi (802.11 b/g/n) |
| Serial Debug | 115200 baud UART via USB for development monitoring |

## **5.2 FreeRTOS Multi-Task Architecture**

The most critical design decision in SafeRide Sentinel is the use of FreeRTOS to split execution across the ESP32's two cores. Running everything in a single sequential loop — the default Arduino approach — causes blocking: a Twilio HTTP call that takes 3 seconds freezes all sensor reads for 3 seconds, meaning alcohol and fire events go undetected during that window.

| Core | Task | Responsibility |
| ----- | ----- | ----- |
| Core 0 | sensorTask | MQ-3 read every 100ms, flame sensor poll, hazard button check |
| Core 0 | relayTask | Relay switching, speed limiting logic, buzzer control |
| Core 1 | gpsTask | Continuous NEO-6M NMEA parsing via TinyGPS++ |
| Core 1 | twilioTask | HTTPS POST to Twilio API when alert flag is set |

## **5.3 Shared State Flags**

Tasks communicate via volatile shared variables. The volatile keyword prevents the compiler from caching these in registers, ensuring each core always reads the latest value written by the other.

| // shared\_state.h   volatile bool alcoholDetected \= false;   // Set by sensorTask, read by relayTask \+ twilioTask volatile bool fireDetected    \= false;   // Set by sensorTask, read by relayTask \+ twilioTask volatile bool sosPressed      \= false;   // Set by sensorTask, read by twilioTask volatile bool gearLocked      \= false;   // Set by relayTask after countdown   volatile float gpsLat \= 0.0;            // Updated by gpsTask every 1 second volatile float gpsLon \= 0.0; volatile float gpsSpeedKmh \= 0.0; |
| :---- |

## **5.4 Task Implementation**

### **Sensor Task (Core 0\)**

| void sensorTask(void \*param) {   while (true) {     // MQ-3 alcohol read     int raw \= analogRead(MQ3\_PIN);       // GPIO 34, ADC1 only     float voltage \= raw \* (5.0 / 4095.0);       if (voltage \> ALCOHOL\_THRESHOLD && \!alcoholDetected) {       alcoholDetected \= true;     }       // Flame sensor     if (digitalRead(FLAME\_PIN) \== HIGH && \!fireDetected) {       fireDetected \= true;     }       // SOS button (active LOW with internal pull-up)     if (digitalRead(SOS\_PIN) \== LOW && \!sosPressed) {       sosPressed \= true;     }       vTaskDelay(100 / portTICK\_PERIOD\_MS); // non-blocking 100ms   } } |
| :---- |

### **GPS Task (Core 1\)**

| void gpsTask(void \*param) {   while (true) {     // Feed all available bytes to TinyGPS++ parser     while (gpsSerial.available()) {       gps.encode(gpsSerial.read());     }       if (gps.location.isValid()) {       gpsLat      \= gps.location.lat();       gpsLon      \= gps.location.lng();       gpsSpeedKmh \= gps.speed.kmph();     }       vTaskDelay(50 / portTICK\_PERIOD\_MS); // 50ms keeps NMEA buffer clear   } } |
| :---- |

### **Twilio Task (Core 1\)**

| void twilioTask(void \*param) {   while (true) {     if (alcoholDetected || fireDetected || sosPressed) {       String type \= fireDetected ? "FIRE" : alcoholDetected ? "ALCOHOL" : "SOS";       String msg  \= "ALERT \[" \+ type \+ "\] Vehicle safety triggered.";       msg \+= " Location: maps.google.com/?q=";       msg \+= String(gpsLat, 4\) \+ "," \+ String(gpsLon, 4);         sendTwilioSMS(msg);   // HTTPS call — blocks this task only         // Reset flags after sending       sosPressed \= false;     }       vTaskDelay(500 / portTICK\_PERIOD\_MS);   } } |
| :---- |

# **6\. Alcohol Detection — Detailed Behaviour**

## **6.1 MQ-3 Sensor Characteristics**

The MQ-3 is a chemi-resistive gas sensor whose internal resistance decreases when it contacts ethanol molecules. It operates on a 5V heater circuit and outputs an analog voltage proportional to ethanol concentration. Key characteristics:

* Operating voltage: 5V VCC (heater), 5V signal rail

* Output range: 0V (clean air) to 5V (high alcohol concentration)

* Warm-up time: 60 seconds minimum after power-on for stable readings

* Detection range: \~25 ppm to 500 ppm ethanol

* Response time: under 10 seconds

| Threshold Setting The detection threshold is set at 1.8V in firmware (ALCOHOL\_THRESHOLD define in config.h). This corresponds to a breath alcohol concentration roughly equivalent to the Indian legal limit of 30mg/100ml blood alcohol. This value must be re-calibrated for each deployment using reference gas or a commercial breathalyzer for comparison. |
| :---- |

## **6.2 False Positive Mitigation**

The MQ-3 detects ambient ethanol, not specifically breath. The following strategies are implemented to reduce false positives from passengers, spills, perfume, or hand sanitizer:

| Strategy | Implementation |
| ----- | ----- |
| Sensor placement | Mounted on driver-side steering column, not cabin centre — maximises sensitivity to driver breath over ambient sources |
| Ignition-window sampling | Active detection only in the 3-second window after ignition key is turned — sharp breath spike vs flat ambient background |
| Delta-based triggering | System reads ambient baseline on door unlock. Triggers only if reading exceeds baseline by more than the threshold delta, not just absolute voltage |
| Consecutive sample confirmation | Requires 5 consecutive readings above threshold before triggering — eliminates single-sample spikes from brief exposure |
| Firmware threshold tuning | Threshold is a \#define constant — can be adjusted per vehicle type and environment without hardware changes |

## **6.3 Graduated Speed-Lock Sequence**

The 10-second graduated response is the key innovation over simple ignition cutoff. It prevents sudden stops on highways while still enforcing safe speed limits:

| Second | Speed (approx) | Gear Equivalent | Action |
| ----- | ----- | ----- | ----- |
| T+0 | 80 km/h | 4th | Alcohol detected — countdown begins, buzzer alert, SMS sent |
| T+3 | 60 km/h | 3rd | Speed reducing — relay limiting motor output |
| T+6 | 40 km/h | 2nd | Continued reduction |
| T+9 | 28 km/h | 1st (entering) | Almost locked |
| T+10 | 25 km/h max | 1st LOCKED | Hard speed cap — relay holds — re-test required to unlock |
| T+10 min | 25 km/h max | 1st LOCKED | Re-test available — pass unlocks full speed |

# **7\. Fire Detection**

## **7.1 Flame Sensor Characteristics**

The flame sensor module uses a photodiode sensitive to infrared wavelengths in the 760–1100nm range — the primary emission spectrum of open flames. It provides a digital output (HIGH on fire detected) and operates at 3.3V logic, compatible directly with ESP32 GPIO pins.

* Detection angle: approximately 60 degrees

* Detection range: up to 100cm depending on flame intensity

* Response time: under 5ms

* Output: digital HIGH when flame IR exceeds threshold (potentiometer adjustable on module)

## **7.2 Fire Response Sequence**

16. Flame sensor GPIO 25 goes HIGH

17. fireDetected flag set in sensorTask

18. relayTask reads flag — immediately opens relay (cuts ignition/motor)

19. Buzzer activates with rapid fire-pattern beeping

20. twilioTask reads flag — sends emergency SMS with GPS coordinates

21. System remains in fire-alert state until manual reset

| Why Cut Ignition on Fire? Cutting the ignition removes the electrical load and fuel pump power, which are the two primary accelerants of vehicle electrical fires. This is standard procedure in motorsport fire suppression systems and is the most effective single automated action available without a dedicated suppression system. |
| :---- |

# **8\. GPS & Communication**

## **8.1 NEO-6M GPS Module**

The NEO-6M provides NMEA 0183 sentences over UART at 9600 baud. TinyGPS++ decodes these sentences to extract latitude, longitude, altitude, speed, and satellite count. The GPS task runs continuously on Core 1 to ensure no NMEA bytes are lost.

* Acquisition time: 26 seconds (cold start), 1 second (hot start)

* Accuracy: 2.5m CEP horizontal position accuracy

* Update rate: 1Hz (1 position fix per second)

* Satellite systems: GPS

## **8.2 Twilio SMS Integration**

SafeRide Sentinel uses Twilio's REST API to send SMS alerts. The ESP32 connects via WiFi, makes an HTTPS POST request to the Twilio Messages endpoint, and receives a confirmation response. Because this call runs on Core 1, it does not block sensor reads on Core 0\.

| // Twilio SMS via HTTPClient void sendTwilioSMS(String messageBody) {   HTTPClient http;   String url \= "https://api.twilio.com/2010-04-01/Accounts/";   url \+= TWILIO\_SID;   url \+= "/Messages.json";     http.begin(url);   http.setAuthorization(TWILIO\_SID, TWILIO\_TOKEN);   http.addHeader("Content-Type", "application/x-www-form-urlencoded");     String body \= "To=" \+ String(ALERT\_TO);   body \+= "\&From=" \+ String(TWILIO\_FROM);   body \+= "\&Body=" \+ messageBody;     int code \= http.POST(body);   http.end(); } |
| :---- |

## **8.3 SMS Alert Format**

Each alert SMS contains the event type, sensor reading, and a Google Maps link with live coordinates:

| ALERT \[ALCOHOL\] SafeRide Sentinel triggered. Sensor: MQ-3 reading 4.40V (threshold 1.8V) Vehicle speed locked to 25 km/h. Location: maps.google.com/?q=28.6139,77.2090 Time: 14:32:07 Contact authorities if no response. |
| :---- |

# **9\. Implementation Roadmap**

| Phase | Name | Activities |
| ----- | ----- | ----- |
| 1 | Requirement Analysis | Define sensor thresholds, GPIO mapping, alert logic, power budget. Review ESP32 ADC limitations with WiFi. |
| 2 | Hardware Setup | Assemble ESP32, MQ-3, flame sensor, NEO-6M, relay, buzzer, LED and SOS button on breadboard. Verify 5V and 3.3V rails. |
| 3 | Individual Module Testing | Test each module independently: MQ-3 analog output, flame sensor digital output, GPS fix acquisition, relay switching, Twilio API call. |
| 4 | FreeRTOS Integration | Split code into four tasks across two cores. Introduce volatile shared flags. Replace all delay() with vTaskDelay(). |
| 5 | Software Logic | Implement alcohol countdown sequence, gear lock timer, re-test flow, fire cutoff logic, and SOS button handler. |
| 6 | Full System Testing | Test all modules running simultaneously. Verify no blocking. Test edge cases: fire during alcohol lock, SOS during fire alert. |
| 7 | Calibration | Tune MQ-3 threshold against known BAC reference. Adjust flame sensor potentiometer for target range. Validate GPS accuracy. |
| 8 | Deployment & Validation | Install in vehicle. Run real-world tests. Validate SMS delivery time, GPS link accuracy, relay response speed. |

# **10\. Innovation & Differentiation**

## **10.1 What Makes SafeRide Sentinel Different**

| Feature | SafeRide Sentinel | Typical Solutions |
| ----- | ----- | ----- |
| Response type | Graduated — slow then lock | Binary — cut or ignore |
| Driver impact | Safe deceleration, stays in control | Sudden stop risk on highways |
| Fire \+ Alcohol | Both in one system | Separate products only |
| Location sharing | Automatic GPS SMS on trigger | Manual call only |
| Re-test unlock | Yes — driver can prove sobriety | No — permanent lockout |
| Architecture | FreeRTOS dual-core — no blocking | Single loop — blocking delays |
| Cost | Under Rs. 1,500 in components | Commercial: Rs. 20,000+ |
| Scalability | Fleet-ready, mass manufacturable | Specialist installation required |

## **10.2 Production-Grade Improvements (Future Scope)**

* Replace WiFi/Twilio with GSM module (SIM7600) for cellular SMS independent of WiFi — works anywhere with mobile signal

* Add OBD-II interface for actual ECU gear and speed data rather than relay-based limiting

* Camera-based driver drowsiness and distraction detection using ESP32-CAM

* Cloud dashboard for fleet managers with real-time vehicle status and alert history

* Delta-based alcohol detection (baseline on unlock, trigger on spike) for production false-positive reduction

* Tamper detection — alert if sensor wiring is disconnected

# **11\. Team — SafeRide Engineers**

| Name | Role | Contribution |
| ----- | ----- | ----- |
| Priyanshu Bhaskar | Hardware Lead | ESP32 programming, FreeRTOS task architecture, relay and motor control logic |
| Somya Agarwal | Sensor Engineer | MQ-3 calibration, flame sensor integration, threshold logic and false-positive mitigation |
| Preeti Patel | Communications Lead | NEO-6M GPS integration, TinyGPS++ parsing, Twilio SMS API implementation |
| Raj Sisoudia | Systems & Testing | Circuit design, wiring schematic, full system integration testing and debugging |

# **12\. Appendix — Quick Reference**

## **12.1 Pin Configuration Summary**

| // config.h — SafeRide Sentinel v2.0   \#define MQ3\_PIN        34    // ADC1 only — DO NOT use ADC2 pins with WiFi \#define FLAME\_PIN      25 \#define GPS\_RX\_PIN     16 \#define GPS\_TX\_PIN     17 \#define RELAY\_PIN      23 \#define BUZZER\_PIN     22 \#define LED\_PIN        21 \#define SOS\_PIN        18    // Internal pull-up, active LOW   \#define ALCOHOL\_THRESHOLD   1.8f   // Volts — calibrate per deployment \#define GEAR\_LOCK\_SECONDS   600    // 10 minutes \#define SLOWDOWN\_SECONDS    10 \#define MAX\_LOCKED\_SPEED    25     // km/h   \#define WIFI\_SSID      "your\_wifi" \#define WIFI\_PASSWORD  "your\_password" \#define TWILIO\_SID     "ACxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" \#define TWILIO\_TOKEN   "your\_auth\_token" \#define TWILIO\_FROM    "+1XXXXXXXXXX" \#define ALERT\_TO       "+91XXXXXXXXXX" |
| :---- |

## **12.2 Libraries Required**

| Library | Version | Source |
| ----- | ----- | ----- |
| TinyGPS++ | 1.0.3+ | Arduino Library Manager — Mikal Hart |
| WiFi | Built-in | ESP32 Arduino Core |
| HTTPClient | Built-in | ESP32 Arduino Core |
| FreeRTOS | Built-in | ESP32 Arduino Core — no install needed |
| HardwareSerial | Built-in | ESP32 Arduino Core |

## **12.3 Serial Monitor Boot Sequence**

Expected output at 115200 baud after power-on:

| \[BOOT\] ESP32 initializing... \[WIFI\] Connecting to network... \[WIFI\] Connected. IP: 192.168.1.xx \[GPS\]  NEO-6M module OK — awaiting fix \[MQ3\]  Sensor warm-up started (60s) \[RLY\]  Relay ON — engine enabled \[SYS\]  FreeRTOS tasks started on Core 0 \+ Core 1 \[MQ3\]  Warm-up complete — monitoring active \[GPS\]  Fix acquired — 7 satellites — 28.6139N, 77.2090E \[SYS\]  All systems nominal |
| :---- |

| Project Repository Source code, circuit diagrams, and the browser-based 3D simulation are available in the project repository. The simulation (saferide\_simulation.html) demonstrates all system behaviours including alcohol countdown, fire detection, and GPS tracking without requiring physical hardware. |
| :---- |

*— End of Document —*

SafeRide Engineers  |  SCHI103  |  Innovate Bharat Hackathon 2026  |  Sharda University
