#include "oled.h"
#include "config.h"
#include "debug.h"
#include "communications.h"
#include "mqtt_client.h"
#include "sensors.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Dimensões do display OLED
#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool oledAvailable = false;
static int currentPage = 0;
static unsigned long lastPageChange = 0;

// Ícones 16x16 pixels incorporados diretamente no oled.cpp
static const  unsigned char PROGMEM wifiIcon[] = {

	0b00000000, 0b00000000, //                 
	0b00000111, 0b11100000, //      ######     
	0b00011111, 0b11111000, //    ##########   
	0b00111111, 0b11111100, //   ############  
	0b01110000, 0b00001110, //  ###        ### 
	0b01100111, 0b11100110, //  ##  ######  ## 
	0b00001111, 0b11110000, //     ########    
	0b00011000, 0b00011000, //    ##      ##   
	0b00000011, 0b11000000, //       ####      
	0b00000111, 0b11100000, //      ######     
	0b00000100, 0b00100000, //      #    #     
	0b00000001, 0b10000000, //        ##       
	0b00000001, 0b10000000, //        ##       
	0b00000000, 0b00000000, //                 
	0b00000000, 0b00000000, //                 
	0b00000000, 0b00000000, //                 
};

static const  unsigned char PROGMEM apIcon[] = {
	0b00000111, 0b11100000, //      ######      
	0b00001111, 0b11110000, //     ########     
	0b00011111, 0b11111000, //    ##########   
	0b00111111, 0b11111100, //   ############  
	0b01111111, 0b11111110, //  ############## 
	0b11111111, 0b11111111, // ################
	0b11000000, 0b00000011, // ##            ##
	0b11000000, 0b00000011, // ##            ##
	0b11000000, 0b00000011, // ##            ##
	0b11001111, 0b11110011, // ##  ########  ##
	0b11001111, 0b11110011, // ##  ########  ##
	0b11001100, 0b00110011, // ##  ##    ##  ##
	0b11001100, 0b00110011, // ##  ##    ##  ##
	0b11001100, 0b00110011, // ##  ##    ##  ##
	0b11111100, 0b00111111, // ######    ######
	0b11111100, 0b00111111, // ######    ######
};

static const unsigned char PROGMEM mqttConnectedIcon[] ={
	0b00111111, 0b11111100, //   ############  
	0b01111111, 0b11111110, //  ############## 
	0b11111111, 0b11111111, // ################
	0b11110000, 0b00001111, // ####        ####
	0b11100000, 0b00000111, // ###          ###
	0b11100000, 0b00000111, // ###          ###
	0b11100000, 0b00000111, // ###          ###
	0b11100000, 0b00000111, // ###          ###
	0b11100000, 0b00000111, // ###          ###
	0b11100000, 0b00000111, // ###          ###
	0b11110000, 0b00001111, // ####        ####
	0b11110001, 0b11111110, // ####   ######## 
	0b01111011, 0b11111100, //  #### ########  
	0b00111111, 0b11111000, //   ###########   
	0b00011110, 0b00000000, //    ####         
	0b00001100, 0b00000000, //     ##          
};

static const unsigned char PROGMEM mqttDisconnectedIcon [] = {
  0x00,
  0x00,
  0x00,
  0x00,
  0x11,
  0x78,
  0x19,
  0x48,
  0x15,
  0x48,
  0x13,
  0x48,
  0x11,
  0x78,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x8b,
  0xbf,
  0xda,
  0x92,
  0xaa,
  0x92,
  0x8b,
  0xd2,
  0x00,
  0x00,
  0x00,
  0x00
};

static const unsigned char PROGMEM placeholderIcon[] = {
	0b00000000, 0b00000000, //                 
	0b00000000, 0b00000000, //                 
	0b00000000, 0b00000000, //                 
	0b00000000, 0b00000000, //                 
	0b00000000, 0b00000000, //              
	0b00000000, 0b00000000, //          
	0b00000000, 0b00000000, //          
	0b00000000, 0b00000000, //        
	0b00000000, 0b00000000, //        
	0b00000000, 0b00000000, //        
	0b00000000, 0b00000000, //          
	0b00000000, 0b00000000, //          
	0b00000000, 0b00000000, //              
	0b00000000, 0b00000000, //                 
	0b00000000, 0b00000000, //                 
	0b00000000, 0b00000000, //   
};

static const  unsigned char PROGMEM fastModeIcon[] = {
	0b10000000, 0b10000001, // #       #      #
	0b01000000, 0b10000010, //  #      #     # 
	0b00100000, 0b10000100, //   #     #    #  
	0b00010011, 0b11101000, //    #  ##### #   
	0b00000111, 0b11110000, //      #######    
	0b00001111, 0b11111000, //     #########   
	0b00001111, 0b10111000, //     ##### ###   
	0b00011111, 0b10011100, //    ######  ###  
	0b00011111, 0b10011100, //    ######  ###  
	0b00011111, 0b10001100, //    ######   ##  
	0b00111111, 0b10001110, //   #######   ### 
	0b00111111, 0b10001110, //   #######   ### 
	0b01111111, 0b11111111, //  ###############
	0b01111111, 0b11111111, //  ###############
	0b01111111, 0b11111111, //  ###############
	0b00000000, 0b00000000, //                 
};

// **Inicializa o OLED corretamente**
void initOLED() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    WARNING_PRINTLN("OLED não encontrado. Redirecionando interface para Serial.");
    oledAvailable = false;
    return;
  }

  oledAvailable = true;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE); // **Corrigido para garantir texto visível**
  display.display();
}

// Desenha os ícones no canto superior direito do OLED
void drawIcons() {
  display.drawBitmap(112, 0, isWiFiConnected() ? wifiIcon : apIcon, 16, 16, SSD1306_WHITE);
  display.drawBitmap(96, 0, mqttConnected() ? mqttConnectedIcon : mqttDisconnectedIcon, 16, 16, SSD1306_WHITE);
  display.drawBitmap(80, 0, placeholderIcon, 16, 16, SSD1306_WHITE);
  display.drawBitmap(64, 0, sensorFastMode ? fastModeIcon : placeholderIcon, 16, 16, SSD1306_WHITE);
}

// Obtém a hora atual em formato `HH:MM`
String getCurrentTime() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    return String(timeStr);
  }
  return "Indisponível"; // **Corrigido para exibir "Indisponível" quando não há sincronização**
}

void displayPage(int page, const char* message, const char* mqttMsg) {
  if (!oledAvailable) return;

  display.clearDisplay();
  drawIcons();

  if (page == 0) { // Página de Rede
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.print("SSID: ");
    display.println(currentSSID);
    display.print("IP: ");
    display.println(currentIP);
    display.print("Hora: ");
    display.println(timeIsValid ? getCurrentTime() : "Indisponível"); // **Exibe hora sincronizada ou "Indisponível"**
  }
  else if (page == 1) { // Página de Sensores
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println(message);
  }
  else if (page == 2) { // Página MQTT
    display.setCursor(0, 20);
    display.println("Último comando MQTT:");
    display.println(mqttConnected() ? mqttMsg : "MQTT Indisponível"); // **Corrigido para mostrar "MQTT Indisponível" quando offline**
  }
  else if (page == 3) { // Página de Mensagens
    display.setCursor(0, 20);
    display.println("Mensagem:");
    display.println(message);
  }

  display.display();
}


void nextPage() {
  currentPage = (currentPage + 1) % 4;
  displayPage(currentPage, "", getLastMqttMessage());
}
