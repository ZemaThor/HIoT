#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

extern WiFiClientSecure secured_client;
extern UniversalTelegramBot bot;

void initTelegram();
void handleTelegramMessages();
bool sendTelegramMessage(const String &message);
void sendSensorReadings(float temp, float hum, const String &node);
void checkDailyReport(const String &node);

#endif