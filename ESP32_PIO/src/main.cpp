#include <Arduino.h>
#include <ArduinoOTA.h>

#include "config.h"
#include "communication.h"

unsigned long lastPageChange = 0;

void setup() {
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    lastPageChange = millis();
    DEBUG_PRINTLN("Booting...");
    //Start the network interface if possible
    DEBUG_PRINTLN("Network Setup starting...");
    initNetwork();

}

void loop() {
  ArduinoOTA.handle();
}