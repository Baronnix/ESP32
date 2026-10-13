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

#define NUMBER_NOTES 128

// --- USB MIDI ---
USBMIDI MIDI;

// --- AsyncWebServer ---
AsyncWebServer server(80);

// --- Variables WiFi ---
String wifiSSID = "";
String wifiPASS = "";

// Notes are in the order of the Midi commmands: the index of the note in the aray is the midi note
String notes[NUMBER_NOTES] = {"C-2", "CS-2", "D-2", "DS-2", "E-2", "F-2", "FS-2", "G-2", "GS-2", "A-2", "AS-2", "B-2", "C-1", "CS-1", "D-1", "DS-1", "E-1", "F-1", "FS-1", "G-1", "GS-1", "A-1", "AS-1", "B-1", "C-0", "CS-0", "D-0", "DS-0", "E-0", "F-0", "FS-0", "G-0", "GS-0", "A-0", "AS-0", "B-0", "C1", "CS1", "D1", "DS1", "E1", "F1", "FS1", "G1", "GS1", "A1", "AS1", "B1", "C2", "CS2", "D2", "DS2", "E2", "F2", "FS2", "G2", "GS2", "A2", "AS2", "B2", "C3", "CS3", "D3", "DS3", "E3", "F3", "FS3", "G3", "GS3", "A3", "AS3", "B3", "C4", "CS4", "D4", "DS4", "E4", "F4", "FS4", "G4", "GS4", "A4", "AS4", "B4", "C5", "CS5", "D5", "DS5", "E5", "F5", "FS5", "G5", "GS5", "A5", "AS5", "B5", "C6", "CS6", "D6", "DS6", "E6", "F6", "FS6", "G6", "GS6", "A6", "AS6", "B6", "C7", "CS7", "D7", "DS7", "E7", "F7", "FS7", "G7"};

String prefix_ssid_ap = "Midi_Controller_AP_";
const char* password_ap = "CROQUETTE";

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

void clearWifiSettingsFromEeprom()
{
  for (int i = 0; i < EEPROM_SIZE; i++) { 
  //String eraseValue = "";
    EEPROM.write(i, 0); 
  }
  EEPROM.commit();
  delay(500);
}

// ------------------------------------------------------------
//                      Access Point
// ------------------------------------------------------------
void startAccessPoint() {
  ledBlue();

  int accessPointNumber = 1;
  int n = WiFi.scanNetworks();
  if (n > 0)
  {
    for (int i = 0; i < n; ++i)
    {
      if (WiFi.SSID(i).substring(0, prefix_ssid_ap.length()) == prefix_ssid_ap && WiFi.SSID(i).substring(WiFi.SSID(i).length()-1, WiFi.SSID(i).length()).toInt() >= accessPointNumber)
      {
        accessPointNumber = WiFi.SSID(i).substring(WiFi.SSID(i).length()-1, WiFi.SSID(i).length()).toInt() + 1;
      }
    }
  }

  String ssid_ap_str = prefix_ssid_ap + String(accessPointNumber);
  char ssid_ap[ssid_ap_str.length()+1];
  ssid_ap_str.toCharArray(ssid_ap, ssid_ap_str.length()+1);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_ap, password_ap);

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
//                      List WiFi
// ------------------------------------------------------------
String sendWifiAvailable() {
  String json = "[";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i)
    {
      if (i > 0 ) {
        json += ",";
      }
      json += "{\"ssid\":\"" + WiFi.SSID(i) + "\"}";
    }
  json += "]";
  return json;
}

// ------------------------------------------------------------
//                      Notes Methods
// ------------------------------------------------------------
void playNote(String note, int channel){
  for(int i=0;i<NUMBER_NOTES;i++)
  {
    if(notes[i] == note) 
    {
      MIDI.noteOn(i, 127, channel); // Play note, vélocité 127, canal
      delay(100); // Attendre 100 ms
      MIDI.noteOff(i, 0, channel);
    }
  }
}

bool isValidNote(String note)
{
  bool result=false;
  for(int i=0;i<NUMBER_NOTES;i++)
  {
    if(notes[i] == note) 
    {
      result = true;
    }
  }
  return result;
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
  server.on("/playNote", HTTP_GET, [](AsyncWebServerRequest *request){
    String result = "Note non supportee.";
    if (!request->hasParam("note") || !request->hasParam("channel")) {
      request->send(400, "text/plain", "Note ou channel non provisionne");
      return;
    }

    String note = request->getParam("note")->value();
    int channel = request->getParam("channel")->value().toInt();

    if (isValidNote(note)) {
      playNote(note,channel);
      result = "Note envoyee";
    }
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

    request->send(200, "text/plain", "Wifi enregistre, pour vous connecter redemarrez l'appareil.");
  });

  server.on("/actions/", HTTP_GET, [](AsyncWebServerRequest *request){
    String result = "Action non supportee";
    if (request->hasParam("actionCode")) {
      String action= request->getParam("actionCode")->value();
      if (action == "reboot") {
        ESP.restart();
        result = "Rebooting";      
      }
      else {
        if (action == "reinitialisation") {
          clearWifiSettingsFromEeprom();
          result = "Reseau Wifi oublie, pour vous deconnecter redemarrez l'appareil."; 
        }
      } 
    }  
    request->send(200, "text/plain", result);
  });

  server.on("/getAvailableWifi", HTTP_GET, [](AsyncWebServerRequest *request){
      String result = sendWifiAvailable();
      request->send(200, "application/json", result);
    });  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.on("/html/menu.html", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/html/menu.html");
    });
  server.on("/html/dashboard.html", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/html/dashboard.html");
    });
  server.on("/html/wifiSettings.html", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/html/wifiSettings.html");
    });
  server.on("/html/wifiStatus.html", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/html/wifiStatus.html");
    });
  server.on("/css/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/css/bootstrap.min.css");
    });
  server.on("/css/gridstack.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/css/gridstack.min.css");
    });
  server.on("/css/globals.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/css/globals.css");
    });
  // On ESP32 not sure why but .js are not working in folders
  server.on("/js/functions.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/js/functions.js");
    });
  server.on("/js/functions-gridstack.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/js/functions-gridstack.js");
    });
  server.on("/js/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/js/bootstrap.min.js");
    });
  server.on("/js/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/js/bootstrap.bundle.min.js");
    });
  server.on("/js/bootstrap-autocomplete.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/js/bootstrap-autocomplete.min.js");
    });
  server.on("/js/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/js/jquery.min.js");
    });
  server.on("/js/gridstack-all.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/js/gridstack-all.min.js");
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
