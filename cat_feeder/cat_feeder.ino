#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <Wire.h>
#include <U8g2lib.h>

#define MOTOR_PIN 3
#define SENSOR_PIN 4
#define BOOT_BUTTON 9
#define SDA_PIN 5
#define SCL_PIN 6

U8G2_SSD1306_72X40_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);
WebServer server(80);
Preferences preferences;

String lastFed = "Never";
bool feeding = false;

void updateOLED(String line1, String line2) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tr);
  display.drawStr(0, 12, line1.c_str());
  display.drawStr(0, 28, line2.c_str());
  display.sendBuffer();
}

void feed() {
  if (feeding) return;
  feeding = true;
  Serial.println("Feeding...");
  updateOLED("Feeding...", "");
  digitalWrite(MOTOR_PIN, HIGH);
  delay(200);
  unsigned long timeout = millis() + 5000;
  while (digitalRead(SENSOR_PIN) == HIGH) {
    if (millis() > timeout) {
      Serial.println("Timeout!");
      break;
    }
  }
  digitalWrite(MOTOR_PIN, LOW);
  lastFed = String(millis() / 60000) + "min ago";
  Serial.println("Fed!");
  updateOLED("Fed!", lastFed);
  delay(2000);
  feeding = false;
}

void handleRoot() {
  String html = "";
  html += "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Cat Feeder</title>";
  html += "<style>";
  html += "body{font-family:sans-serif;display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100vh;margin:0;background:#1a1a2e;color:white;}";
  html += "h1{font-size:2em;margin-bottom:0.2em;}";
  html += "p{color:#aaa;margin-bottom:2em;}";
  html += "button{background:#e94560;color:white;border:none;padding:20px 50px;font-size:1.5em;border-radius:12px;cursor:pointer;}";
  html += "button:active{background:#c73652;}";
  html += ".last{margin-top:2em;color:#aaa;font-size:0.9em;}";
  html += "</style></head><body>";
  html += "<h1>Cat Feeder</h1>";
  html += "<p>Press to dispense one portion</p>";
  html += "<button onclick=\"fetch('/feed').then(function(){document.querySelector('.last').innerText='Feeding...';setTimeout(function(){location.reload();},3000);})\">Feed Now</button>";
  html += "<div class='last'>Last fed: " + lastFed + "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleFeed() {
  feed();
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  delay(6000);  // wait for everything to stabilise on cold boot

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(BOOT_BUTTON, INPUT_PULLUP);
  digitalWrite(MOTOR_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin();
  updateOLED("Starting...", "");

  // Generate random AP name
  String apSSID = "CATFEEDER" + String(random(100, 999));

  WiFiManager wm;

  // Show AP info on OLED when in config mode
  wm.setAPCallback([apSSID](WiFiManager* wm) {
    updateOLED("AP Mode", apSSID);
  });

  updateOLED("WiFi...", "Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  // autoConnect blocks until connected
  // if no saved credentials, opens AP with given name
  if (!wm.autoConnect(apSSID.c_str())) {
    updateOLED("WiFi Failed!", "Restarting...");
    delay(3000);
    ESP.restart();
  }

  String ip = WiFi.localIP().toString();
  Serial.println("Connected! IP: " + ip);
  updateOLED("Ready!", ip);

  server.on("/", handleRoot);
  server.on("/feed", handleFeed);
  server.begin();
}

void loop() {
  server.handleClient();
  yield();

  // Rotate OLED between IP and last fed
  static unsigned long lastOLEDSwitch = 0;
  static bool showIP = true;
  if (!feeding && millis() - lastOLEDSwitch > 5000) {
    lastOLEDSwitch = millis();
    if (showIP) {
      updateOLED("IP:", WiFi.localIP().toString());
    } else {
      updateOLED("Last fed:", lastFed);
    }
    showIP = !showIP;
  }

  // Boot button
  if (digitalRead(BOOT_BUTTON) == LOW) {
    unsigned long pressStart = millis();
    while (digitalRead(BOOT_BUTTON) == LOW) {
      delay(10);
      if (millis() - pressStart >= 10000) {
        updateOLED("Resetting", "WiFi...");
        delay(1000);
        WiFiManager wm;
        wm.resetSettings();
        ESP.restart();
      }
    }
    if (millis() - pressStart < 10000) {
      feed();
    }
  }
}