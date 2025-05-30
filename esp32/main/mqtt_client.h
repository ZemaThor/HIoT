#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <PubSubClient.h>

void mqttInit();
void mqttLoop();
void mqttPublishSensorReading(const char* sensorType, float value);
bool mqttConnected();

// Adicionada função global para acessar a última mensagem MQTT
const char* getLastMqttMessage();

#endif // MQTT_CLIENT_H
