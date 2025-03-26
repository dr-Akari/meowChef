#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "wifiManager.h"
#include "captiveportal.h"
#include "controlpage.h"
#include "systemutils.h"
// #include "esp_task_wdt.h"  // For watchdog functions

#define MDNS_HOSTNAME "meowchef"
#define WATCHDOG_TIMEOUT 30  // seconds

void setup() {
  Serial.begin(115200);
  delay(1000); // Allow time for Serial monitor to start
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  configTime(19800, 0, ntpServer);
  // // Initialize the watchdog timer using the proper configuration structure.
  // esp_task_wdt_config_t wdt_config = {
  //   .timeout_ms = WATCHDOG_TIMEOUT * 1000,   // 30 seconds in milliseconds.
  //   .idle_core_mask = (1 << 0) | (1 << 1),     // Monitor idle tasks on both cores.
  //   .trigger_panic = true                     // Trigger panic on timeout.
  // };
  // esp_task_wdt_init(&wdt_config);
  // esp_task_wdt_add(NULL);

  // Try to load WiFi credentials from NVS.
  String ssid, pwd;
  if (!loadWifiCredentials(ssid, pwd)) {
    Serial.println("No WiFi credentials found. Starting captive portal.");
    captivePortal();  // Blocks while in captive portal mode.
  } else {
    Serial.println("WiFi credentials found. Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pwd.c_str());

    // Attempt connection with a timeout (20 seconds)
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
      delay(500);
      Serial.print(".");
      // esp_task_wdt_reset(); // Reset the watchdog during connection attempt.
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nFailed to connect to WiFi. Starting captive portal.");
      captivePortal();
    }
    
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Start mDNS so that meowchef.local is accessible.
    if (!MDNS.begin(MDNS_HOSTNAME)) {
      Serial.println("Error setting up mDNS responder!");
    } else {
      Serial.println("mDNS responder started as " + String(MDNS_HOSTNAME) + ".local");
    }

    // Load any saved dispensing schedule from NVS.
    loadSchedule();

    // Set up the control page server (this also initializes the servo).
    setupControlPage();
  }
}

void loop() {
  // Reset the watchdog periodically.
  // esp_task_wdt_reset();
  if (WiFi.status() == WL_CONNECTED) {
    controlServer.handleClient();
    
    // Auto-stop the servo after the dispensing duration.
    if (servoActive && millis() >= servoStopTime) {
      mg995Servo.write(neutralPosition);
      servoActive = false;
      Serial.println("Auto-stopped servo after dispensing duration.");
    }
    // Check the schedule every 60 seconds.
    if (millis() - lastScheduleCheck >= 60000) {
      lastScheduleCheck = millis();
      checkSchedule();
    }

    // Check for reset button press to clear credentials.
    checkResetButton();
  }
  yield();
}
