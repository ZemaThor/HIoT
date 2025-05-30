#include "mqtt_client.h"
#include "debug.h"
#include "config.h"
#include "sensors.h"
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "user_interface.h"

// Cliente Wi‑Fi para comunicação MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Obtém variáveis globais do sistema
extern char macSuffix[7];   // Identificador do ESP
extern bool isAccessPoint;  // Modo AP (sem MQTT)

// Estado de conexão MQTT
static int mqttFailedAttempts = 0;
static unsigned long lastMqttAttempt = 0;

// Última mensagem MQTT recebida
static char lastMqttMessage[64] = "";

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (isAccessPoint) return;  // Ignora MQTT se estiver em modo AP

    char msg[length + 1];
    memcpy(msg, payload, length);
    msg[length] = '\0';

    snprintf(lastMqttMessage, sizeof(lastMqttMessage), "%s", msg); // Armazena a última mensagem recebida

    char expectedTopic[32];
    sprintf(expectedTopic, "/node/esp32-%s", macSuffix);

    if (strcmp(topic, expectedTopic) == 0) {
        if (strcmp(msg, "update") == 0) {
            mqttPublishSensorReading("temperature", readTemperature());
            mqttPublishSensorReading("humidity", readHumidity());
        } else {
            char statusTopic[40];
            sprintf(statusTopic, "/node/status/esp32-%s", macSuffix);
            client.publish(statusTopic, "command not recognized", true);
        }
    }
    /*To show important commands only*/
    if (strcmp(msg, "reset") == 0) {
        uiEnqueueMessage("Remote Reset Command", MessagePriority::PRIO_CRITICAL);
    }
}

void mqttReconnect() {
    if (isAccessPoint || millis() - lastMqttAttempt < 900000UL) return; // 15 min cooldown

    while (!client.connected()) {
        char clientId[16];
        sprintf(clientId, "esp32-%s", macSuffix);

        if (client.connect(clientId)) {
            DEBUG_PRINTLN("MQTT connected");
            char subscribeTopic[32];
            sprintf(subscribeTopic, "/node/esp32-%s", macSuffix);
            client.subscribe(subscribeTopic);
            mqttFailedAttempts = 0;
            uiEnqueueMessage("MQTT Connected", MessagePriority::PRIO_NORMAL);
        } else {
            mqttFailedAttempts++;
            DEBUG_PRINTLN("MQTT fail: " + mqttFailedAttempts);
            uiEnqueueMessageF(MessagePriority::PRIO_HIGH, 
                   "MQTT Fail %d/5", mqttFailedAttempts);
        }
        if (mqttFailedAttempts >= 5) {
            lastMqttAttempt = millis();
                return;
        }
        delay(5000);
    }

}

void mqttInit() {
    if (isAccessPoint) return;
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);
    mqttReconnect();
}

void mqttLoop() {
    if (!client.connected()) {
        mqttReconnect();
    }
    client.loop();
}

bool mqttConnected() {
    return client.connected() && !isAccessPoint;
}

const char* getLastMqttMessage() {
    return lastMqttMessage;
}

void mqttPublishSensorReading(const char* sensorType, float value) {
    char timeStr[20] = "N/A";
    time_t now = time(nullptr);
    if (now > 100000) {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                     timeinfo.tm_year + 1900,
                     timeinfo.tm_mon + 1,
                     timeinfo.tm_mday,
                     timeinfo.tm_hour,
                     timeinfo.tm_min,
                     timeinfo.tm_sec);
        }
    }

    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"esp\":\"esp32-%s\",\"datetime\":\"%s\",\"type\":\"%s\",\"value\":%.2f}",
             macSuffix, timeStr, sensorType, value);

    if (client.publish(MQTT_TOPIC, payload, true)) {
        DEBUG_PRINT("MQTT publish ok: ");
        DEBUG_PRINTLN(payload);
    } else {
        DEBUG_PRINTLN("Falha ao publicar no MQTT");
        uiEnqueueMessage("Falha ao publicar no MQTT", MessagePriority::PRIO_HIGH);        
    }
}
