#include "USB.h"
#include "USBMIDI.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

// --- LED RGB intégrée ESP32-S3 DevKitC-1 ---
#define LED_PIN 48
#define LED_COUNT 1
Adafruit_NeoPixel rgbLED(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- EEPROM ---
#define EEPROM_SIZE 200
#define SSID_ADDR   0
#define PASS_ADDR   100

// --- USB MIDI ---
USBMIDI MIDI;

// --- WebServer ---
WebServer server(80);

// --- Variables WiFi ---
String wifiSSID = "";
String wifiPASS = "";

// ------------------------------------------------------------
//                      LED RGB FUNCTIONS
// ------------------------------------------------------------
void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  rgbLED.setPixelColor(0, rgbLED.Color(r, g, b));
  rgbLED.show();
}

void ledBlue() { setLEDColor(0, 0, 255); }
void ledOrange() { setLEDColor(255, 80, 0); }
void ledOff() { setLEDColor(0, 0, 0); }

// ------------------------------------------------------------
//                      EEPROM FUNCTIONS
// ------------------------------------------------------------
String readEEPROM(int addr, int maxLen) {
  String value = "";
  for (int i = 0; i < maxLen; i++) {
    char c = EEPROM.read(addr + i);
    if (c == 0xFF || c == 0) break;
    value += c;
  }
  return value;
}

void writeEEPROM(int addr, const String &value, int maxLen) {
  for (int i = 0; i < maxLen; i++) {
    if (i < value.length()) EEPROM.write(addr + i, value[i]);
    else EEPROM.write(addr + i, 0);
  }
  EEPROM.commit();
}

// ------------------------------------------------------------
//                      API /setWifi
// ------------------------------------------------------------
void handleSetWifi() {
  if (!server.hasArg("ssid") || !server.hasArg("password")) {
    server.send(400, "text/plain", "Missing ssid or password");
    return;
  }

  String newSSID = server.arg("ssid");
  String newPASS = server.arg("password");

  writeEEPROM(SSID_ADDR, newSSID, 100);
  writeEEPROM(PASS_ADDR, newPASS, 100);

  server.send(200, "text/plain", "WiFi credentials saved. Reboot ESP32.");

  Serial.println("🔧 Nouveau SSID enregistré : " + newSSID);
  Serial.println("🔧 Nouveau PASS enregistré : " + newPASS);
}

// ------------------------------------------------------------
//                      API /playNote
// ------------------------------------------------------------
void handlePlayNote() {
  if (!server.hasArg("note") || !server.hasArg("channel")) {
    server.send(400, "text/plain", "Missing note or channel");
    return;
  }

  int note = server.arg("note").toInt();
  int channel = server.arg("channel").toInt();

  MIDI.noteOn(note, 100, channel);
  delay(200);
  MIDI.noteOff(note, 0, channel);

  server.send(200, "text/plain", "Note played");
}

// ------------------------------------------------------------
//                      Serve index.html
// ------------------------------------------------------------
void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "index.html missing");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

// ------------------------------------------------------------
//                      Access Point Mode
// ------------------------------------------------------------
void startAccessPoint() {
  ledBlue();  // LED BLEUE = AP

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Setup", "12345678");

  Serial.println("📡 Mode Access Point activé !");
  Serial.println("SSID : ESP32-Setup");
  Serial.println("PASS : 12345678");
  Serial.println("URL : http://192.168.4.1/");
}

// ------------------------------------------------------------
//                      WiFi Connection Attempt
// ------------------------------------------------------------
bool tryConnectWiFi() {
  ledOrange();  // LED ORANGE = tentative WiFi

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPASS.c_str());

  Serial.println("⏳ Tentative de connexion WiFi pendant 30 secondes...");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 30000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("📶 WiFi connecté !");
    Serial.println(WiFi.localIP());
    ledOff();  // LED OFF = connecté
    return true;
  }

  Serial.println("❌ Échec WiFi → passage en Access Point");
  return false;
}

// ------------------------------------------------------------
//                      SETUP
// ------------------------------------------------------------
void setup() {
  USB.begin();
  Serial.begin(115200);
  delay(1000);

  // LED RGB
  rgbLED.begin();
  rgbLED.setBrightness(50);
  ledOff();

  Serial.println("=== USB MIDI + WebServer + mDNS + LittleFS + EEPROM + RGB LED ===");

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("❌ Erreur LittleFS");
    return;
  }

  // Lecture EEPROM
  wifiSSID = readEEPROM(SSID_ADDR, 100);
  wifiPASS = readEEPROM(PASS_ADDR, 100);

  Serial.println("📁 EEPROM SSID : " + wifiSSID);
  Serial.println("📁 EEPROM PASS : " + wifiPASS);

  bool hasCredentials = wifiSSID.length() > 0 && wifiPASS.length() > 0;

  // Boot logic
  if (!hasCredentials) {
    Serial.println("⚠️ Aucun WiFi en EEPROM → mode AP");
    startAccessPoint();
  } else {
    if (!tryConnectWiFi()) {
      startAccessPoint();
    }
  }

  // mDNS
  if (MDNS.begin("midiserver")) {
    Serial.println("📛 mDNS actif : midiserver.local");
  }

  // Routes
  server.on("/", handleRoot);
  server.on("/playNote", handlePlayNote);
  server.on("/setWifi", handleSetWifi);

  server.begin();
  Serial.println("🚀 Serveur Web démarré !");
}

// ------------------------------------------------------------
//                      LOOP
// ------------------------------------------------------------
void loop() {
  server.handleClient();
}
