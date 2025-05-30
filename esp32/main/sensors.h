#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Sensor constants
#define TEMP_INVALID -127.0f
#define HUM_INVALID -1.0f

void initSensors();
float readTemperature(bool force = false);  // Added force read parameter
float readHumidity(bool force = false);     // Added force read parameter
void sensorTask();
bool sensorDataAvailable();                 // New function to check data validity

extern bool sensorFastMode;
extern unsigned long lastSensorRead;        // Make this available for diagnostics

#endif // SENSORS_H