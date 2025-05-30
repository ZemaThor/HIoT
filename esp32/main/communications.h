#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

#include <Arduino.h>

void initNetwork();
void networkTask();
bool isWiFiConnected(); // Restaurada a função para verificação da conexão Wi-Fi

extern char currentSSID[32];
extern char currentIP[16];

extern char macSuffix[7];  
extern bool isAccessPoint;
extern bool timeIsValid;
extern unsigned long lastWifiReconnectAttempt;

#endif // COMMUNICATIONS_H
