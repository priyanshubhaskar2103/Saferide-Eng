// ============================================================
//   ESP32 Vehicle Safety System — FINAL
//   Step 1: Power ON  → Motor runs
//   Step 2: SOS Switch ON → Hazard + Alert
//   Step 3: Fire detected → Hazard + Alert
//   Step 4: Alcohol detected → Hazard + Alert
//   All clear → Motor ON, everything OFF
// ============================================================

#include <WiFi.h>
#include <TinyGPS++.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// Load all sensitive configurations from external file (NOT tracked by Git)
#include "secrets.h"

// ─── Telegram ───────────────────────────────────────────────
WiFiClientSecure tlsClient;
UniversalTelegramBot bot(BOT_TOKEN, tlsClient);

// ─── GPS ────────────────────────────────────────────────────
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

// ─── Pins ───────────────────────────────────────────────────
#define FLAME   34    // Flame sensor  DO → D34
#define MQ3     35    // Alcohol sensor AO → D35
#define BUZZER  25    // Buzzer         +  → D25
#define RELAY   26    // Relay          S  → D26
#define LED     27    // LED            +  → D27
#define SWITCH  14    // SOS Switch    COM → D14, other pin → GND

// ─── Alcohol threshold ──────────────────────────────────────
const int ALCOHOL_THRESHOLD = 2000;

// ─── Alcohol hold (stays active 5s after raw value drops) ───
bool          alcoholActive = false;
unsigned long alcoholTimer  = 0;
const int     ALCOHOL_HOLD  = 5000;

// ─── Alert flag (sent once, re-arms when danger clears) ─────
bool alertSent = false;

// ─── Hazard blink timing ────────────────────────────────────
unsigned long previousMillis = 0;
const int     BLINK_MS       = 300;
bool          blinkState     = false;

// ════════════════════════════════════════════════════════════
//  SMS via Twilio
// ════════════════════════════════════════════════════════════
void sendSMS(String message, String toNumber) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  WiFiClientSecure smsClient;
  smsClient.setInsecure();
  String url = "https://api.twilio.com/2010-04-01/Accounts/"
               + String(ACCOUNT_SID) + "/Messages.json";
  http.begin(smsClient, url);
  http.setAuthorization(ACCOUNT_SID, AUTH_TOKEN);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String body = "To="    + toNumber          +
                "&From=" + String(FROM_NUMBER) +
                "&Body=" + message;
  http.POST(body);
  http.end();
}

// ════════════════════════════════════════════════════════════
//  Send Telegram + SMS to both numbers
// ════════════════════════════════════════════════════════════
void sendAllAlerts(String message) {
  float lat = 0, lng = 0;
  if (gps.location.isValid()) {
    lat = gps.location.lat();
    lng = gps.location.lng();
  }

  String fullMsg  = message;
         fullMsg += "\n📍 Location:\nhttps://maps.google.com/?q=";
         fullMsg += String(lat, 6) + "," + String(lng, 6);

  bot.sendMessage(CHAT_ID, fullMsg, "");
  sendSMS(fullMsg, num1);
  sendSMS(fullMsg, num2);
}

// ════════════════════════════════════════════════════════════
//  Hazard mode — blink LED + Buzzer together
// ════════════════════════════════════════════════════════════
void runHazard() {
  unsigned long now = millis();
  if (now - previousMillis >= BLINK_MS) {
    previousMillis = now;
    blinkState     = !blinkState;
    digitalWrite(LED,    blinkState);
    digitalWrite(BUZZER, blinkState);
  }
}

// ════════════════════════════════════════════════════════════
//  Normal mode — motor ON, everything else OFF
// ════════════════════════════════════════════════════════════
void normalMode() {
  digitalWrite(RELAY,  HIGH);   // Motor ON
  digitalWrite(LED,    LOW);
  digitalWrite(BUZZER, LOW);
  blinkState = false;
  alertSent  = false;           // re-arm for next event
}

// ════════════════════════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  pinMode(FLAME,  INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY,  OUTPUT);
  pinMode(LED,    OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);   // COM=D14, other pin=GND

  // ── STEP 1: Power ON → Motor ON immediately ─────────────
  digitalWrite(RELAY,  HIGH);      // Motor ON
  digitalWrite(BUZZER, LOW);
  digitalWrite(LED,    LOW);

  // GPS
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected ✅");

  tlsClient.setInsecure();

  Serial.println("System ready — Motor running.");
}

// ════════════════════════════════════════════════════════════
//  MAIN LOOP
// ════════════════════════════════════════════════════════════
void loop() {

  // Feed GPS parser
  while (gpsSerial.available()) gps.encode(gpsSerial.read());

  // Read all inputs
  bool sosON         = (digitalRead(SWITCH) == LOW);   // button pressed
  bool fireDetected  = (digitalRead(FLAME)  == 0);     // DO=0 means fire
  int  alcoholRaw    = analogRead(MQ3);

  // Alcohol hold logic
  if (alcoholRaw > ALCOHOL_THRESHOLD) {
    alcoholActive = true;
    alcoholTimer  = millis();
  }
  if (alcoholActive && (millis() - alcoholTimer > ALCOHOL_HOLD)) {
    alcoholActive = false;
  }

  // ══════════════════════════════════════════════════════════
  //  STEP 2 — SOS Switch pressed (highest priority)
  // ══════════════════════════════════════════════════════════
  if (sosON) {
    digitalWrite(RELAY, LOW);     // Motor STOP
    runHazard();                  // LED blink + Buzzer beep

    if (!alertSent) {
      sendAllAlerts(
        "🆘 EMERGENCY ALERT!\n"
        "SOS switch activated by driver."
      );
      alertSent = true;
    }
  }

  // ══════════════════════════════════════════════════════════
  //  STEP 3 — Fire detected
  // ══════════════════════════════════════════════════════════
  else if (fireDetected) {
    digitalWrite(RELAY, LOW);     // Motor STOP
    runHazard();                  // LED blink + Buzzer beep

    if (!alertSent) {
      sendAllAlerts(
        "🔥 ALERT! Fire Detected!\n"
        "Vehicle safety system triggered."
      );
      alertSent = true;
    }
  }

  // ══════════════════════════════════════════════════════════
  //  STEP 4 — Alcohol detected
  // ══════════════════════════════════════════════════════════
  else if (alcoholActive) {
    digitalWrite(RELAY, LOW);     // Motor STOP
    runHazard();                  // LED blink + Buzzer beep

    if (!alertSent) {
      sendAllAlerts(
        "🍺 ALERT! Alcohol Detected!\n"
        "Driver may be intoxicated."
      );
      alertSent = true;
    }
  }

  // ══════════════════════════════════════════════════════════
  //  ALL CLEAR — Nothing detected, normal operation
  // ══════════════════════════════════════════════════════════
  else {
    normalMode();                 // Motor ON, LED OFF, Buzzer OFF
  }

  delay(300);
}