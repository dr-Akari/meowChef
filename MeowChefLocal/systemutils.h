#ifndef SYSTEMUTILS_H
#define SYSTEMUTILS_H

#include <nvs_flash.h>
#include <Arduino.h>
#include "Config.h"  // Ensure you define RESET_BUTTON_PIN here or adjust as needed

// Erase NVS credentials (clears WiFi credentials stored)
inline void resetCredentials() {
  Serial.println("Erasing NVS credentials...");
  nvs_flash_erase();
  nvs_flash_init();
  Serial.println("Credentials cleared. Restarting...");
  delay(2000);
  ESP.restart();
}

// Check if the reset button is pressed (with long press detection)
inline void checkResetButton() {
  if(digitalRead(RESET_BUTTON_PIN) == LOW) {
    unsigned long pressStart = millis();
    while(digitalRead(RESET_BUTTON_PIN) == LOW) {
      delay(10);
      if(millis() - pressStart > 3000) { // 3-second long press.
        Serial.println("Long press detected on RESET button. Resetting credentials.");
        resetCredentials();
        break;
      }
    }
  }
}

#endif // SYSTEMUTILS_H
