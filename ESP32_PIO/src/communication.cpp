#include "communication.h"
#include "config.h"
#include <WiFi.h>
#include <time.h>
#include <ArduinoOTA.h>

char currentSSID[32] = "ESP-NO-SSID";
char currentIP[16] = "0.0.0.0";
char macSuffix[7] = {0};
char hostname[16] = "ESP32NOMAC";

bool isAccessPoint = true;
bool timeIsValid = false;
unsigned long lastWifiReconnectAttempt = 0;

void updateMacSuffix() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String suffix = mac.substring(mac.length() - 6);
  suffix.toUpperCase();
  suffix.toCharArray(macSuffix, 7);
  sprintf(hostname, "ESP32-%s", macSuffix);
}

void syncTimeWithNTP() {
  if (isAccessPoint) return;

  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // Basic NTP config
  
  // Lisbon Timezone (Portugal)
  // Format: "STD<offset>DST<offset>,start[/time],end[/time]"
  setenv("TZ", "WET0WEST-1,M3.5.0/1:00:00,M10.5.0/2:00:00", 1);
  tzset();

  unsigned long ntpStart = millis();
  while (time(nullptr) < 100000 && millis() - ntpStart < 10000) {
    delay(500);
  }
  
  timeIsValid = true;
  // Verification code (optional)
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){
    Serial.printf("Lisbon Time: %02d:%02d:%02d DST: %d\n",
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec,
                 timeinfo.tm_isdst);
  }
}

void initNetwork() {
  /*Define OTA Stuff*/
  ArduinoOTA
  .onStart([]() {
    DEBUG_PRINTLN("OTA Update starting");
    //uiEnqueueMessage("OTA Update Starting", MessagePriority::PRIO_HIGH);
  })
  .onEnd([]() {
    DEBUG_PRINTLN("OTA Update complete");
    //uiEnqueueMessage("OTA Update Complete", MessagePriority::PRIO_HIGH);
  })
  .onError([](ota_error_t error) {
    DEBUG_PRINTLN("OTA Error");
    //uiEnqueueMessageF(MessagePriority::PRIO_CRITICAL, 
    //                 "OTA Error %d", error);
  });


  updateMacSuffix();
  WiFi.setHostname(hostname);

  WiFi.mode(WIFI_STA);
  for (int attempt = 0; attempt < 3; attempt++) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
      delay(500);
      DEBUG_PRINTLN("Connect attempt");
    }
    WiFi.onEvent([](WiFiEvent_t event) {
  switch(event) {
    case WIFI_EVENT_STA_CONNECTED:
      DEBUG_PRINTLN("Wifi Connected");  
      // uiEnqueueMessage("WiFi Connected", MessagePriority::PRIO_NORMAL);
      break;
    case WIFI_EVENT_STA_DISCONNECTED:
      DEBUG_PRINTLN("WiFi Disconnected");
      // uiEnqueueMessage("WiFi Disconnected", MessagePriority::PRIO_HIGH);
      break;
    case IP_EVENT_STA_GOT_IP:
      DEBUG_PRINTLN("Got IP");

      //  uiEnqueueMessageF(MessagePriority::PRIO_NORMAL, 
      //                 "IP: %s", WiFi.localIP().toString().c_str());
      break;
  }
});

    if (WiFi.status() == WL_CONNECTED) {
      DEBUG_PRINTLN("Wifi connected");
      // uiEnqueueMessage("WiFi Connected", MessagePriority::PRIO_LOW);
      isAccessPoint = false;
      syncTimeWithNTP();
      snprintf(currentSSID, sizeof(currentSSID), "%s", WiFi.SSID().c_str());
      snprintf(currentIP, sizeof(currentIP), "%s", WiFi.localIP().toString().c_str());
      lastWifiReconnectAttempt = millis();
      ArduinoOTA.begin();
      return;
    }
  }

  isAccessPoint = true;
  WiFi.mode(WIFI_AP);
  DEBUG_PRINTLN("AP connected");
  // uiEnqueueMessage("AP Mode Active", MessagePriority::PRIO_HIGH);
  snprintf(currentSSID, sizeof(currentSSID), "AP Mode");
  snprintf(currentIP, sizeof(currentIP), "N/A");
  lastWifiReconnectAttempt = millis();
  timeIsValid = false;
  ArduinoOTA.begin();
}
