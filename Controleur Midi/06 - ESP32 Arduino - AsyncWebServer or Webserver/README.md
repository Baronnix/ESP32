# 🎛️ Utiliser AsyncWebServer au lieu de WebServer pour le projet USB‑MIDI ESP32‑S3

Ce tutoriel explique étape par étape comment convertir un projet ESP32‑S3 utilisant WebServer vers une architecture moderne et non‑bloquante basée sur AsyncWebServer.

Tu y apprendras comment adapter les routes API (/playNote, /setWifi), servir des fichiers depuis LittleFS, gérer le WiFi et l’Access Point, conserver les identifiants dans l’EEPROM, activer mDNS, et maintenir la stabilité du système tout en utilisant USB MIDI.

Le guide inclut un code final complet, prêt à compiler, pour obtenir un serveur web asynchrone performant, compatible avec les projets temps‑réel comme les contrôleurs MIDI.

# 📺 Vidéo

Lien Youtube: [https://www.youtube.com/@Baronnix/playlists](https://www.youtube.com/@Baronnix/playlists)

# 📦 Prérequis

* Carte ESP32‑S2 / ESP32‑S3 compatible TinyUSB (Dans ce tutoriel on utilise la carte ESP32‑S3 DevKit C1)
* Câble USB‑C
* Un ordinateur (tutoriel réalisé sur Windows)
* Arduino IDE installé
* esp32 installé dans le gestionnaire de cartes (esp32 par Espressif Systems - 3.3.10)

Ce tutoriel repart du code final du tutoriel [05 - ESP32 Arduino - USB‑C MIDI Controleur - EEPROM with Access Point and Built-in LED](https://github.com/Baronnix/ESP32/tree/main/Controleur%20Midi/05%20-%20ESP32%20Arduino%20-%20USB%E2%80%91C%20MIDI%20Controleur%20-%20EEPROM%20with%20Access%20Point%20and%20Built-in%20LED)

# ⚖️ Comparaison WebServer vs AsyncWebServer

## 🟥 WebServer — modèle synchrone (bloquant)

Le serveur classique WebServer fonctionne de manière séquentielle :
* Il traite une seule requête à la fois
* Il dépend de server.handleClient() dans loop()
* Toute requête lente (WiFi, fichier, client lent) bloque le reste du programme
* Le code USB MIDI, les animations LED, ou les tâches temps‑réel peuvent geler
* Risque de WDT reset si une requête prend trop de temps
* Impossible d’utiliser WebSerial ou WebSocket correctement
* Mauvaise gestion de plusieurs clients simultanés

👉 Adapté uniquement pour des projets très simples, sans timing critique.

## 🟩 AsyncWebServer — modèle asynchrone (non‑bloquant)

AsyncWebServer repose sur AsyncTCP et un modèle événementiel :
* Aucune fonction bloquante dans loop()
* Le serveur tourne sur un autre cœur du CPU (Core 0)
* Le code USB MIDI reste fluide sur Core 1
* Support natif de WebSerial, WebSocket, SSE
* Capable de gérer 10 à 20 clients simultanés
* Streaming de fichiers LittleFS ultra rapide
* Aucune latence dans les API /playNote ou /setWifi
* Stabilité accrue en mode AP ou STA

👉 Parfait pour les projets temps‑réel, interactifs, ou multi‑clients.

## 🎯 Rationnel : pourquoi changer?

Le projet combine :
* USB MIDI (temps‑réel, sensible à la latence)
* Web UI (index.html + API)
* WiFi STA + AP
* EEPROM + LittleFS
* mDNS
* LED RGB
* Futur WebSerial / WebSocket

Avec WebServer, tu risques :
* des blocages lors du chargement de fichiers HTML
* des latences MIDI lors des requêtes HTTP
* des freezes si un client se déconnecte mal
* des WDT resets si une requête prend trop de temps
* une interface web lente ou non réactive
* des problèmes de concurrence si plusieurs appareils se connectent

Avec AsyncWebServer, tu obtiens :
* un serveur non‑bloquant
* un ESP32‑S3 ultra stable, même sous charge
* des API instantanées pour jouer des notes MIDI
* une interface web fluide, même en AP
* la possibilité d’ajouter WebSerial ou WebSocket sans rien casser
* une architecture moderne, robuste, scalable

👉 Pour un contrôleur MIDI + interface web, le choix n’est pas juste “meilleur”, il est obligatoire si tu veux un système fiable.

# 🎯 Objectif du tutoriel

Passer de :
* WebServer server(80);
* server.handleClient();
* Handlers synchrones (bloquants)

À :
* AsyncWebServer server(80);
* Handlers asynchrones
* Aucun blocage dans loop()
* Meilleure stabilité pour USB MIDI + WiFi + Web UI

# 🧩 Étape 1 — Ajouter les bonnes librairies

Remplacer :
```cpp
#include <WebServer.h>
```
par :
```cpp
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
```

# 🧩 Étape 2 — Remplacer WebServer par AsyncWebServer

Remplacer :
```cpp
WebServer server(80);
```
par :
```cpp
AsyncWebServer server(80);
```

# 🧩 Étape 3 — Adapter les handlers

## API /playNote en Async

Remplacer :
```cpp
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
```
et:
```cpp
server.on("/playNote", handlePlayNote);
```
par :
```cpp
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
```

## API /setWifi en Async

Remplacer :
```cpp
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
```
et:
```cpp
server.on("/setWifi", handleSetWifi);
```
par :
```cpp
server.on("/setWifi", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->hasParam("ssid") || !request->hasParam("password")) {
        request->send(400, "text/plain", "Missing ssid or password");
        return;
    }

    String newSSID = request->getParam("ssid")->value();
    String newPASS = request->getParam("password")->value();

    Serial.println("🔧 Nouveau SSID enregistré : " + newSSID);
    Serial.println("🔧 Nouveau PASS enregistré : " + newPASS);

    writeEEPROM(SSID_ADDR, newSSID, 100);
    writeEEPROM(PASS_ADDR, newPASS, 100);

    request->send(200, "text/plain", "WiFi credentials saved. Reboot ESP32.");
});
```

# 🧩 Étape 4 — Servir index.html avec AsyncWebServer

Remplacer :
```cpp
server.on("/", handleRoot);
```
par :
```cpp
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
});
```

# 🧩 Étape 5 — Supprimer server.handleClient()

Avec AsyncWebServer aucune action n'est nécessaire dans loop()
```cpp
void loop() {
    // rien
}
```

# 🧩 Étape 6 — Démarrer le serveur

Aucun changement nécessaire pour démarrer le serveur Web:
```cpp
server.begin();
```

# 🧩 Étape 7 — Boot logic WiFi / AP inchangé

On garde:
* Lecture EEPROM
* Tentative WiFi 30 sec
* LED orange / bleu / off
* Mode AP si échec

AsyncWebServer fonctionne parfaitement en AP ou STA.