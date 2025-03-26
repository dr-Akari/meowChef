#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Preferences.h>

Preferences preferences;

// Save WiFi credentials to NVS
bool saveWifiCredentials(const char* ssid, const char* pwd) {
  preferences.begin("wifiCreds", false);
  bool ssidWrite = preferences.putString("ssid", ssid);
  bool pwdWrite = preferences.putString("pwd", pwd);
  preferences.end();
  return ssidWrite && pwdWrite;
}

// Load WiFi credentials from NVS
bool loadWifiCredentials(String &ssid, String &pwd) {
  preferences.begin("wifiCreds", true);
  ssid = preferences.getString("ssid", "");
  pwd = preferences.getString("pwd", "");
  preferences.end();
  return ssid.length() > 0;
}

#endif // WIFIMANAGER_H
