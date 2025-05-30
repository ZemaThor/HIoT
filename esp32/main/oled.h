#ifndef OLED_H
#define OLED_H

#include <Arduino.h>

extern bool oledAvailable;

// Inicializa o OLED
void initOLED();

// Exibe uma página específica
void displayPage(int page, const char* message, const char* mqttMsg);

// Alterna para a próxima página (chamada automática ou via botão)
void nextPage();

#endif // OLED_H
