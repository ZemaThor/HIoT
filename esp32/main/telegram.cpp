#include "telegram.h"
#include "config.h"
#include "sensors.h"
#include <time.h>
#include <WiFi.h>

WiFiClientSecure myhttpclient;
float minTemp = 100.0, maxTemp = -100.0;
float minHum = 100.0, maxHum = 0.0;
time_t lastResetTime = 0;

void initTelegram() {
  myhttpclient.setInsecure(); // For simple TLS (adjust for production)
}

bool sendTelegramMessage(const char* message) {
  if(!WiFi.isConnected()) return false;

  HTTPClient https;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) + "/sendMessage";
  
  https.begin(myhttpclient, url);
  https.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(256);
  doc["chat_id"] = TELEGRAM_CHAT_ID;
  doc["text"] = message;
  
  String payload;
  serializeJson(doc, payload);
  
  int code = https.POST(payload);
  https.end();
  
  return (code == 200);
}

void updateMinMax(float temp, float hum) {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) return;

  // Reset min/max at midnight
  if(timeinfo.tm_hour == 0 && timeinfo.tm_min == 0) {
    minTemp = temp; maxTemp = temp;
    minHum = hum; maxHum = hum;
    return;
  }

  if(temp < minTemp) minTemp = temp;
  if(temp > maxTemp) maxTemp = temp;
  if(hum < minHum) minHum = hum;
  if(hum > maxHum) maxHum = hum;
}

void sendSensorReadings(float temp, float hum) {
  updateMinMax(temp, hum);
  
  char message[128];
  snprintf(message, sizeof(message),
    "🌡️ Current Readings:\n"
    "Temperature: %.1f°C\n"
    "Humidity: %.1f%%",
    temp, hum);
    
  sendTelegramMessage(message);
}

void checkDailyReport() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) return;

  if(timeinfo.tm_hour == DAILY_REPORT_HOUR && timeinfo.tm_min == 0) {
    char report[256];
    snprintf(report, sizeof(report),
      "📊 Daily Report:\n"
      "Max Temp: %.1f°C\n"
      "Min Temp: %.1f°C\n"
      "Max Hum: %.1f%%\n"
      "Min Hum: %.1f%%",
      maxTemp, minTemp, maxHum, minHum);
      
    if(sendTelegramMessage(report)) {
      // Reset after successful send
      minTemp = 100.0; maxTemp = -100.0;
      minHum = 100.0; maxHum = 0.0;
    }
  }
}