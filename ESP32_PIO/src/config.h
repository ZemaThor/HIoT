#ifndef CONFIG_H
#define CONFIG_H

// Message priorities (add this at the top)
enum class MessagePriority {
    PRIO_LOW,      // Normal operational messages (2s)
    PRIO_NORMAL,   // Regular notifications (3s)
    PRIO_HIGH,     // Important alerts (5s)
    PRIO_CRITICAL  // Requires immediate attention (8s)
};

// Definições dos pinos e hardware
#define BUTTON_PIN   0    // Botão ligado ao GPIO0
#define OLED_SDA_PIN 5    // SDA do OLED no GPIO5
#define OLED_SCL_PIN 4    // SCL do OLED no GPIO4
#define DHT11_PIN    15   // Sensor DHT11 ligado ao GPIO15
#define DHTTYPE       DHT11

// Credenciais Wi‑Fi
#define WIFI_SSID     "xxx"
#define WIFI_PASSWORD "xxx"

// Configurações do MQTT
#define MQTT_SERVER "xxxxxx"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "/sensores"

// Telegram Config
#define TELEGRAM_BOT_TOKEN "xxx"
#define TELEGRAM_CHAT_ID "xxx"
#define DAILY_REPORT_HOUR 22 // 10 PM

// Para ativar as mensagens de debug e warning, descomente as linhas abaixo:
//#define DEBUG
//#define WARNING

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#ifdef WARNING
  #define WARNING_PRINT(x)    Serial.print(x)
  #define WARNING_PRINTLN(x)  Serial.println(x)
#else
  #define WARNING_PRINT(x)
  #define WARNING_PRINTLN(x)
#endif

#endif // CONFIG_H
