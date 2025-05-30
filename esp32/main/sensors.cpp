#include "sensors.h"
#include "config.h"
#include "debug.h"
#include "user_interface.h"
#include <Arduino.h>
#include <time.h>
#include <DHT.h>
#include "mqtt_client.h"
#include "telegram.h"
#include "communications.h"

DHT dht(DHT11_PIN, DHTTYPE);

bool sensorFastMode = false;
unsigned long lastSensorRead = 0;
float lastValidTemp = TEMP_INVALID;
float lastValidHum = HUM_INVALID;

static bool sensorButtonPrevious = HIGH;
static unsigned long sensorButtonPressStart = 0;

void initSensors() {
    dht.begin();
    // Initial read to populate values
    lastValidTemp = readTemperature(true);
    lastValidHum = readHumidity(true);
}

float readTemperature(bool force) {
    static unsigned long lastRead = 0;
    const unsigned long minInterval = 2000; // DHT11 needs 2s between reads
    
    if (!force && millis() - lastRead < minInterval) {
        return lastValidTemp; // Return cached value
    }
    
    float temp = dht.readTemperature();
    if (!isnan(temp)) {
        lastValidTemp = temp;
        lastRead = millis();
    }
    return isnan(temp) ? lastValidTemp : temp;
}

float readHumidity(bool force) {
    static unsigned long lastRead = 0;
    const unsigned long minInterval = 2000; // DHT11 needs 2s between reads
    
    if (!force && millis() - lastRead < minInterval) {
        return lastValidHum; // Return cached value
    }
    
    float hum = dht.readHumidity();
    if (!isnan(hum)) {
        lastValidHum = hum;
        lastRead = millis();
    }
    return isnan(hum) ? lastValidHum : hum;
}

bool sensorDataAvailable() {
    return (lastValidTemp != TEMP_INVALID) && 
           (lastValidHum != HUM_INVALID);
}

// Função que gerencia as leituras dos sensores e alternância de estado
void sensorTask() {
    if (!sensorDataAvailable()) {
        DEBUG_PRINTLN("Warning: No valid sensor data yet");
        return;
    }
    //nome do no
      char hostname[16] = {0};
      sprintf(hostname, "esp32-%s", macSuffix);


    // --- Detecção de long press do botão para alternar o modo ---
    bool currentButton = digitalRead(BUTTON_PIN);

    if (sensorButtonPrevious == HIGH && currentButton == LOW) {
        sensorButtonPressStart = millis();
    }

    if (sensorButtonPrevious == LOW && currentButton == LOW) {
        if (millis() - sensorButtonPressStart >= 2000) {
            sensorFastMode = !sensorFastMode;
            uiEnqueueMessage(sensorFastMode ? "Sensor fast mode ON" : "Sensor fast mode OFF", 
                MessagePriority::PRIO_NORMAL);

            DEBUG_PRINTLN(sensorFastMode ? "Sensor fast mode ON" : "Sensor fast mode OFF");

            // Evita bloqueios: aguarda até que o botão seja solto sem usar delay()
            if (digitalRead(BUTTON_PIN) == LOW && millis() - sensorButtonPressStart < 50) {
            // Still pressed
            } else {
                sensorButtonPressStart = millis();
            }   
        }
    }

    sensorButtonPrevious = currentButton;

    // --- Definir o intervalo de leitura baseado na hora do NTP ---
    unsigned long sensorInterval = 15000UL;  // Default: 15s em modo rápido
#ifndef DEBUG
    extern bool timeIsValid;
    if (timeIsValid && !sensorFastMode) {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            int hour = timeinfo.tm_hour;
            int minute = timeinfo.tm_min;

            if ((hour == 7 && minute >= 30) || (hour == 8))
                sensorInterval = 10000UL;  // 10s entre 07:30 e 09:00
            else if (hour >= 9 && hour < 11)
                sensorInterval = 300000UL; // 5 minutos entre 09:00 e 11:00
            else
                sensorInterval = 1200000UL; // 20 minutos nos restantes períodos
        } else {
            sensorInterval = 15000UL; // Se falhar a hora, mantém 15s
        }
    }
#endif

    // --- Efetua a leitura se o intervalo tiver decorrido ---
    if (millis() - lastSensorRead >= sensorInterval) {
        lastSensorRead = millis();
        float temp = readTemperature();
        float hum  = readHumidity();

        //Envia ao telegram
        sendSensorReadings(temp, hum, hostname);
        // Envia os valores via MQTT se estiver conectado
        if (mqttConnected()) {
            mqttPublishSensorReading("temperature", temp);
            mqttPublishSensorReading("humidity", hum);
        } else {
#ifdef DEBUG
            // Se não estiver conectado ao MQTT e estiver em debug, imprime no Serial
            char msg[64];
            snprintf(msg, sizeof(msg), "Temp: %.2f C, Hum: %.2f%%", temp, hum);
            DEBUG_PRINTLN(msg);
#endif
        }
        // Exibe os valores na página do OLED
        uiEnqueueMessageF(MessagePriority::PRIO_NORMAL, "Temp: %.2f°C\nHum: %.2f%%", temp, hum);
    }
    checkDailyReport(hostname); // Add this at end of function

}
