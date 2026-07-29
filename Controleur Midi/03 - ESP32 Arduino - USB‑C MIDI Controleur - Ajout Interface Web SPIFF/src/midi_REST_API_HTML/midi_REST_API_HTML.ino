#include "USB.h"
#include "USBMIDI.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>

USBMIDI MIDI;
WebServer server(80);

// --- CONFIG WIFI ---
const char* ssid = "MonWifi";
const char* password = "MonMotDePasse";

// --- API /playNote ---
void handlePlayNote() {
  if (!server.hasArg("note") || !server.hasArg("channel")) {
    server.send(400, "text/plain", "Missing note or channel");
    Serial.println("⚠️ Requête invalide : paramètres manquants");
    return;
  }

  int note = server.arg("note").toInt();
  int channel = server.arg("channel").toInt();

  Serial.printf("🎵 Lecture note=%d channel=%d\n", note, channel);

  MIDI.noteOn(note, 100, channel);
  delay(200);
  MIDI.noteOff(note, 0, channel);

  server.send(200, "text/plain", "Note played");
}

// --- PAGE WEB ---
void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "index.html missing");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void setup() {
  USB.begin();
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== USB MIDI + WebServer + mDNS + LittleFS ===");

  // --- LittleFS ---
  if (!LittleFS.begin(true)) {
    Serial.println("❌ Erreur LittleFS");
    return;
  }
  Serial.println("📁 LittleFS monté");

  // --- WIFI ---
  Serial.println("Connexion au WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }

  Serial.println("\n📶 WiFi connecté !");
  Serial.print("📡 IP locale : ");
  Serial.println(WiFi.localIP());

  // --- mDNS ---
  if (!MDNS.begin("midiserver")) {
    Serial.println("❌ Erreur : mDNS non démarré");
  } else {
    Serial.println("📛 Nom mDNS actif : midiserver.local");
    Serial.println("🌐 URL API : http://midiserver.local/playNote?note=60&channel=1");
    Serial.println("🌐 Interface : http://midiserver.local/");
  }

  // --- ROUTES ---
  server.on("/", handleRoot);
  server.on("/playNote", handlePlayNote);

  server.begin();
  Serial.println("🚀 Serveur Web démarré !");
}

void loop() {
  server.handleClient();
}
