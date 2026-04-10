// ============================================================
//   ESP32 Vehicle Safety System — SECRETS TEMPLATE
//   Rename this file to "secrets.h" and enter your details
// ============================================================
#ifndef SECRETS_H
#define SECRETS_H

// ─── WiFi ───────────────────────────────────────────────────
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ─── Telegram ───────────────────────────────────────────────
#define BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define CHAT_ID   "YOUR_TELEGRAM_CHAT_ID"

// ─── Twilio SMS ─────────────────────────────────────────────
#define ACCOUNT_SID "YOUR_TWILIO_ACCOUNT_SID"
#define AUTH_TOKEN  "YOUR_TWILIO_AUTH_TOKEN"
#define FROM_NUMBER "YOUR_TWILIO_PHONE_NUMBER"

// ─── Emergency Contacts ─────────────────────────────────────
String num1 = "+1234567890";
String num2 = "+1234567890";

#endif
