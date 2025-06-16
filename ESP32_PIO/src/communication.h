#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>

void initNetwork();
void updateMacSuffix();
void syncTimeWithNTP();

extern char currentSSID[32];
extern char currentIP[16];
extern char macSuffix[7];
extern char hostname[16];


extern bool isAccessPoint;
extern bool timeIsValid;
extern unsigned long lastWifiReconnectAttempt;

#endif // COMMUNICATIONS_H