#ifndef CAPTIVE_H
#define CAPTIVE_H

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "wifiManager.h"

#define CAPTIVE_PORTAL_PORT 80
#define DNS_PORT 53

DNSServer dnsServer;
WebServer server(CAPTIVE_PORTAL_PORT);

// Updated captive portal HTML remains mostly unchanged.
const char* captivePortalHTML = R"HTMLSTRING(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>MeowChef; WiFi Setup</title>
    <style>
      body { font-family: Arial, sans-serif; background: #f7f7f7; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; min-height: 100vh; }
      .container { max-width: 500px; width: 90%; background: #fff; padding: 30px; border-radius: 8px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }
      h2 { color: #333; margin: 0 0 10px; font-size: 1.5em; }
      p.description { color: #555; margin: 0; font-size: 0.95em; }
      form { display: flex; flex-direction: column; }
      label { margin: 10px 0 5px; color: #555; }
      input[type="text"], input[type="password"] { padding: 10px; border: 1px solid #ccc; border-radius: 4px; font-size: 14px; }
      .checkbox-container { display: flex; align-items: center; margin: 10px 0; }
      .checkbox-container input { margin-right: 5px; }
      #passwordError { color: red; font-size: 0.9em; margin-top: 5px; display: none; }
      input[type="submit"] { margin-top: 20px; padding: 12px; background: #fab616; border: none; border-radius: 4px; color: #fff; font-size: 16px; cursor: pointer; transition: background 0.3s ease; }
      input[type="submit"]:hover { background: #d29d23; }
      @media (max-width: 600px) { .container { padding: 20px; } }
    </style>
  </head>
  <body>
    <div class="container">
      <h2>MeowChef; WiFi Setup</h2>
      <p class="description">Please enter the Wi-Fi credentials that the device will use to connect.</p>
      <form action="/save" method="post" onsubmit="return validatePasswords()">
        <label for="ssid">SSID:</label>
        <input type="text" id="ssid" name="ssid" required />
        <label for="pwd">Password:</label>
        <input type="password" id="pwd" name="pwd" required />
        <label for="pwd2">Confirm Password:</label>
        <input type="password" id="pwd2" name="pwd2" required />
        <div id="passwordError">Passwords do not match.</div>
        <div class="checkbox-container">
          <input type="checkbox" id="showPwd" onclick="togglePassword()" />
          <label for="showPwd">Show Password</label>
        </div>
        <input type="submit" value="Save" />
      </form>
    </div>
    <script>
      function togglePassword() {
        var pwd = document.getElementById("pwd");
        var pwd2 = document.getElementById("pwd2");
        var type = pwd.type === "password" ? "text" : "password";
        pwd.type = type;
        pwd2.type = type;
      }
      function validatePasswords() {
        var pwd = document.getElementById("pwd").value;
        var pwd2 = document.getElementById("pwd2").value;
        if (pwd2.length > 0 && pwd !== pwd2) {
          document.getElementById("passwordError").style.display = "block";
          return false;
        } else {
          document.getElementById("passwordError").style.display = "none";
          return true;
        }
      }
      document.getElementById("pwd2").addEventListener("input", function () {
        var pwd = document.getElementById("pwd").value;
        var pwd2 = document.getElementById("pwd2").value;
        if (pwd2.length > 0 && pwd !== pwd2) {
          document.getElementById("passwordError").style.display = "block";
        } else {
          document.getElementById("passwordError").style.display = "none";
        }
      });
    </script>
  </body>
</html>
)HTMLSTRING";

// Handler for root URL; serves the captive portal page.
void handleRoot() {
  server.send(200, "text/html", captivePortalHTML);
}

// Handler for form submission.
void handleSave() {
  if (server.hasArg("ssid") && server.hasArg("pwd")) {
    String ssid = server.arg("ssid");
    String pwd  = server.arg("pwd");

    if (saveWifiCredentials(ssid.c_str(), pwd.c_str())) {
      // Respond with a page that shows an alert and auto-redirects.
      String response = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Credentials Saved</title>";
      response += "<script>";
      response += "alert('Credentials Saved! To easily access your device, add http://meowchef.local to your homescreen.');";
      response += "setTimeout(function(){ window.location.href = 'http://meowchef.local'; }, 5000);";
      response += "</script></head><body><h2>Credentials Saved. Redirecting to control panel...To easily access your device, add http://meowchef.local to your homescreen.</h2></body></html>";
      server.send(200, "text/html", response);
      delay(5000);
      ESP.restart();
    } else {
      server.send(500, "text/html", "<h2>Error saving credentials.</h2>");
    }
  } else {
    server.send(400, "text/html", "<h2>Error: Missing SSID or Password.</h2>");
  }
}

void startCaptivePortal() {
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound([](){
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", "");
  });
  server.begin();
  Serial.println("Captive portal started.");
}

void captivePortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("MeowChef-Captive");
  Serial.print("Captive portal IP: ");
  Serial.println(WiFi.softAPIP());
  startCaptivePortal();
  while(true) {
    dnsServer.processNextRequest();
    server.handleClient();
    delay(10);
  }
}

#endif // CAPTIVE_H
