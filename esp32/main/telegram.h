#ifndef TELEGRAM_CLIENT_H
#define TELEGRAM_CLIENT_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void initTelegram();
bool sendTelegramMessage(const char* message);
void sendSensorReadings(float temp, float hum);
void checkDailyReport();

#endif