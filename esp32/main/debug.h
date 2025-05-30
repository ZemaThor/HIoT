#ifndef DEBUG_H
#define DEBUG_H

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

#endif // DEBUG_H
