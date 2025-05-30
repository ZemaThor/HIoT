#include "telegram.h"
#include "config.h"
#include "sensors.h"
#include <time.h>
#include <WiFi.h>
#include "communications.h"


WiFiClientSecure secured_client;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, secured_client);

// Min/Max tracking variables
float minTemp = 100.0, maxTemp = -100.0;
float minHum = 100.0, maxHum = 0.0;
unsigned long lastBotUpdateTime = 0; // Last time messages were checked

void initTelegram() {
    secured_client.setInsecure();

    //Telegram keyboard Menu
    String keyboardJson = "[[\"/updateALL\", \"/help\"]]";
    bot.setMyCommands(keyboardJson);
}

void handleTelegramMessages() {
  if (millis() - lastBotUpdateTime > 1000) { // Check every 1 second
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;

        if (text == "/updateALL" || text == "/statusALL") {
          // Get fresh readings (bypassing any delays)
          float temp = readTemperature(); // Force new reading
          float hum = readHumidity();
          
            char hostname[16] = {0};
            sprintf(hostname, "esp32-%s", macSuffix);

          sendSensorReadings(temp, hum, hostname);
          
          // Optional: Send min/max too
          String extras = "📊 24h Statistics:\n";
          extras += "Max: " + String(maxTemp,1) + "°C\n";
          extras += "Min: " + String(minTemp,1) + "°C\n";
          bot.sendMessage(chat_id, extras, "");
        }
        else if (text == "/help") {
          String help = "Available commands:\n";
          help += "/updateALL - Current sensors readings\n";
          help += "/statusALL - Same as /updateALL\n";
          help += "/help - This message";
          bot.sendMessage(chat_id, help, "");
        }
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastBotUpdateTime = millis();
  }
}

bool sendTelegramMessage(const String &message) {
  if(!WiFi.isConnected()) return false;
  
  // Using Universal Telegram Bot's built-in method
  return bot.sendMessage(TELEGRAM_CHAT_ID, message, "");
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

void sendSensorReadings(float temp, float hum, const String &node) {
  updateMinMax(temp, hum);
  
  String message = "🌡️ Current Readings from " + node + ":\n";
  message += "Temperature: " + String(temp, 1) + "°C\n";
  message += "Humidity: " + String(hum, 1) + "%";
  
  sendTelegramMessage(message);
}

void checkDailyReport(const String &node) {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) return;

  if(timeinfo.tm_hour == DAILY_REPORT_HOUR && timeinfo.tm_min == 0) {
    String report = "📊 Daily Report from " + node + ":\n";
    report += "Max Temp: " + String(maxTemp, 1) + "°C\n";
    report += "Min Temp: " + String(minTemp, 1) + "°C\n";
    report += "Max Hum: " + String(maxHum, 1) + "%\n";
    report += "Min Hum: " + String(minHum, 1) + "%";
    
    if(sendTelegramMessage(report)) {
      // Reset after successful send
      minTemp = 100.0; maxTemp = -100.0;
      minHum = 100.0; maxHum = 0.0;
    }
  }
}