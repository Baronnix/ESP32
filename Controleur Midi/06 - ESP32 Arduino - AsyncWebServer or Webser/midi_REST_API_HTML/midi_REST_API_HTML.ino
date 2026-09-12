#include "USB.h"
#include "USBMIDI.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

// --- LED RGB ---
#define LED_PIN 48
#define LED_COUNT 1
Adafruit_NeoPixel rgbLED(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- EEPROM ---
#define EEPROM_SIZE 200
#define SSID_ADDR   0
#define PASS_ADDR   100

// --- USB MIDI ---
USBMIDI MIDI;

// --- AsyncWebServer ---
AsyncWebServer server(80);

// --- Variables WiFi ---
String wifiSSID = "";
String wifiPASS = "";

// ------------------------------------------------------------
//                      LED RGB
// ------------------------------------------------------------
void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  rgbLED.setPixelColor(0, rgbLED.Color(r, g, b));
  rgbLED.show();
}

void ledBlue() { setLEDColor(0, 0, 255); }
void ledOrange() { setLEDColor(255, 80, 0); }
void ledOff() { setLEDColor(0, 0, 0); }

// ------------------------------------------------------------
//                      EEPROM
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
//                      Access Point
// ------------------------------------------------------------
void startAccessPoint() {
  ledBlue();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Setup", "12345678");

  Serial.println("📡 Mode AP activé : ESP32-Setup");
}

// ------------------------------------------------------------
//                      WiFi connect
// ------------------------------------------------------------
bool tryConnectWiFi() {
  ledOrange();
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPASS.c_str());

  Serial.println("⏳ Tentative WiFi 30 sec...");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 30000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("📶 WiFi OK : " + WiFi.localIP().toString());
    ledOff();
    return true;
  }

  Serial.println("❌ WiFi FAIL → AP");
  return false;
}

// ------------------------------------------------------------
//                      SETUP
// ------------------------------------------------------------
void setup() {
  USB.begin();
  Serial.begin(115200);
  delay(1000);

  rgbLED.begin();
  rgbLED.setBrightness(50);
  ledOff();

  EEPROM.begin(EEPROM_SIZE);

  if (!LittleFS.begin(true)) {
    Serial.println("❌ Erreur LittleFS");
    return;
  }

  wifiSSID = readEEPROM(SSID_ADDR, 100);
  wifiPASS = readEEPROM(PASS_ADDR, 100);

  bool hasCredentials = wifiSSID.length() > 0 && wifiPASS.length() > 0;

  if (!hasCredentials) startAccessPoint();
  else if (!tryConnectWiFi()) startAccessPoint();

  if (MDNS.begin("midiserver")) {
    Serial.println("📛 mDNS actif : midiserver.local");
  }

  // --- ROUTES ---
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/playNote", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->hasParam("note") || !request->hasParam("channel")) {
      request->send(400, "text/plain", "Missing note or channel");
      return;
    }

    int note = request->getParam("note")->value().toInt();
    int channel = request->getParam("channel")->value().toInt();

    MIDI.noteOn(note, 100, channel);
    delay(200);
    MIDI.noteOff(note, 0, channel);

    request->send(200, "text/plain", "Note played");
  });

  server.on("/setWifi", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->hasParam("ssid") || !request->hasParam("password")) {
      request->send(400, "text/plain", "Missing ssid or password");
      return;
    }

    String newSSID = request->getParam("ssid")->value();
    String newPASS = request->getParam("password")->value();

    writeEEPROM(SSID_ADDR, newSSID, 100);
    writeEEPROM(PASS_ADDR, newPASS, 100);

    request->send(200, "text/plain", "WiFi credentials saved. Reboot ESP32.");
  });

  server.begin();
  Serial.println("🚀 AsyncWebServer démarré !");
}

// ------------------------------------------------------------
//                      LOOP
// ------------------------------------------------------------
void loop() {
  // Async = rien ici
}
