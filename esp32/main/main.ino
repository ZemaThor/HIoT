#include "config.h"
#include "debug.h"
#include "oled.h"
#include "sensors.h"
#include "communications.h"
#include "mqtt_client.h"
#include "user_interface.h"
#include "telegram.h"
#include <ArduinoOTA.h>

extern char lastMessage[64];
extern const char* getLastMqttMessage();

unsigned long lastPageChange = 0;
unsigned long lastButtonCheck = 0;
const unsigned long buttonCheckInterval = 100;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  initOLED();
  initSensors();
  initNetwork();
  mqttInit();
  initTelegram();
  sendTelegramMessage("🔔 ESP32 Online!");

  lastPageChange = millis();
  uiEnqueueMessage("System Booting...", MessagePriority::PRIO_LOW);
  displayPage(0, "", ""); 
}

void loop() {
  networkTask();
  mqttLoop();
  sensorTask();
  uiProcessQueue(); // Add this line

  // Replace your message timeout check with:
  if (millis() - lastPageChange >= 3000 && !uiIsMessageActive()) {
    lastPageChange = millis();
    nextPage();
  }

  // Button handling remains the same
  if (millis() - lastButtonCheck >= buttonCheckInterval) {
    lastButtonCheck = millis();
    if (digitalRead(BUTTON_PIN) == LOW) {
      unsigned long buttonPressStart = millis();
      while (digitalRead(BUTTON_PIN) == LOW && millis() - buttonPressStart < 50);
      nextPage();
    }
  }

  if (ESP.getFreeHeap() < 10000) {
    uiEnqueueMessageF(MessagePriority::PRIO_HIGH, 
                     "Low Memory: %d bytes", ESP.getFreeHeap());
  }

  handleTelegramMessages();
  ArduinoOTA.handle();

}
