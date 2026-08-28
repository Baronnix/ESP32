# 🎹 Ajouter WiFi dynamique, EEPROM, Access Point et LED au projet USB‑MIDI ESP32‑S3

Ce tutoriel explique comment transformer un simple projet USB MIDI + WebServer en un système complet capable de :
* Stocker et modifier les identifiants WiFi via une API REST
* Démarrer automatiquement en mode Station ou Access Point
* Utiliser la LED intégrée pour indiquer l’état du système
* Gérer un fallback WiFi automatique
* Tester l’ensemble étape par étape

Ce guide est conçu pour être suivi progressivement, en partant du code initial jusqu’au code final.

# 📺 Vidéo

Lien Youtube: [https://www.youtube.com/@Baronnix/playlists](https://www.youtube.com/@Baronnix/playlists)

# 📦 Prérequis

* Carte ESP32‑S2 / ESP32‑S3 compatible TinyUSB (Dans ce tutoriel on utilise la carte ESP32‑S3 DevKit C1)
* Câble USB‑C
* Un ordinateur (tutoriel réalisé sur Windows)
* Arduino IDE installé
* esp32 installé dans le gestionnaire de cartes (esp32 par Espressif Systems - 3.3.10)

Ce tutoriel repart du code final du tutoriel [03 - ESP32 Arduino - USB‑C MIDI Controleur - Ajout Interface Web SPIFF](https://github.com/Baronnix/ESP32/tree/main/Controleur%20Midi/03%20-%20ESP32%20Arduino%20-%20USB%E2%80%91C%20MIDI%20Controleur%20-%20Ajout%20Interface%20Web%20SPIFF)

Lien Youtube tutoriel précédent: [https://www.youtube.com/watch?v=PnMmg08P6Bs](https://www.youtube.com/watch?v=PnMmg08P6Bs)

# Dependances et Versions

* Arduino IDE - 2.3.10
* Arduino Plugins:
  * arduino-littlefs-upload - 1.6.3
* Cartes:
  * esp32 par Espressif Systems - 3.3.11
* Librairies
  * Adafruit NeoPixel - 1.15.5

# 1. 🧭 Comportement global du système (preambule)

Avant d’entrer dans les détails techniques, voici le comportement final recherché :

Au démarrage :
* Le programme lit le SSID et le mot de passe dans l’EEPROM.
* Si les deux champs sont vides :
  * L’ESP32 démarre en Access Point
  * La LED devient bleue
* Si les champs sont remplis :
  * L’ESP32 tente de se connecter au WiFi pendant 30 secondes
  * Pendant la tentative, la LED devient orange
  * Si la connexion réussit :
    * La LED s’éteint
  * Si la connexion échoue :
    * L’ESP32 bascule en Access Point
    * La LED devient bleue

API disponible :
* /playNote : jouer une note MIDI
* /setWifi?ssid=XXX&password=YYY : enregistrer un nouveau WiFi dans l’EEPROM

# 2. 🏁 Étape 1 — Code initial : USB MIDI + WebServer

Le projet de départ contient :
* Un serveur Web simple
* Une API /playNote
* Un fichier index.html servi via LittleFS
* Une connexion WiFi fixe, codée en dur

Cette base fonctionne, mais ne permet pas :
* De changer le WiFi sans recompiler
* De gérer les erreurs de connexion
* De fonctionner en mode Access Point
* D’indiquer l’état via une LED

# 3. 🔧 Étape 2 — Ajout de l’EEPROM pour stocker le WiFi

L’objectif est de permettre à l’utilisateur de changer le SSID et le mot de passe sans recompiler le firmware.

## Pourquoi l’EEPROM ?

L’EEPROM permet de stocker des données persistantes, même après redémarrage.
Parfait pour enregistrer un SSID et un mot de passe WiFi.

## Ce que nous ajoutons :

* Une fonction pour lire une chaîne depuis l’EEPROM
* Une fonction pour écrire une chaîne dans l’EEPROM
* Deux zones mémoire :
  * SSID à l’adresse 0
  * Mot de passe à l’adresse 100

## API ajoutée : /setWifi

Elle permet de mettre à jour les identifiants WiFi :

```bash
/setWifi?ssid=MonWifi&password=MonMotDePasse
```

## Effet :

* Les identifiants sont enregistrés dans l’EEPROM
* Un redémarrage est nécessaire pour les appliquer

## Etapes: 

### 3.1 — Initialiser l’EEPROM

Ajoute en haut du fichier :
```cpp
#include <EEPROM.h>

#define EEPROM_SIZE 200
#define SSID_ADDR   0
#define PASS_ADDR   100
```
Explication:
* L’EEPROM est une petite zone mémoire persistante.
* On réserve 200 octets.
* On stocke le SSID à partir de l’adresse 0.
* On stocke le mot de passe à partir de l’adresse 100.

Dans setup() :
```cpp
EEPROM.begin(EEPROM_SIZE);
```
* Démarre l'utilisation de la mémoire et indique la taille utilisée.

### 3.2 — Lire une chaîne depuis l’EEPROM

Ajoute :
```cpp
String readEEPROM(int addr, int maxLen) {
  String value = "";
  for (int i = 0; i < maxLen; i++) {
    char c = EEPROM.read(addr + i);
    if (c == 0xFF || c == 0) break;
    value += c;
  }
  return value;
}
```
Explication:
* On lit caractère par caractère.
* On s’arrête si la mémoire est vide (0xFF) ou si on rencontre un \0.

### 3.3 — Écrire une chaîne dans l’EEPROM

Ajoute :
```cpp
void writeEEPROM(int addr, const String &value, int maxLen) {
  for (int i = 0; i < maxLen; i++) {
    if (i < value.length()) EEPROM.write(addr + i, value[i]);
    else EEPROM.write(addr + i, 0);
  }
  EEPROM.commit();
}
```
Explication:
* On écrit chaque caractère.
* On remplit le reste avec des 0.
* EEPROM.commit() sauvegarde physiquement.

### 3.4 — Ajouter l’API /setWifi

Dans les routes, ajoute :
```cpp
server.on("/setWifi", handleSetWifi);
```

Ajoute aussi la fonction :
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
}
```
Explication:
* L’API reçoit ssid et password.
* Les valeurs sont écrites dans l’EEPROM.
* Un reboot est nécessaire pour les appliquer.


# 4. 📡 Étape 3 — Mode Access Point et fallback automatique

L’objectif est de garantir que l’ESP32 reste accessible même si le WiFi échoue.

## Pourquoi un Access Point ?

Si l’utilisateur entre un mauvais mot de passe, ou si le réseau n’est pas disponible, l’ESP32 doit rester accessible.

Le mode AP garantit que l’utilisateur peut toujours :
* Se connecter à l’ESP32
* Modifier les identifiants WiFi
* Accéder à l’interface web

## Comportement ajouté :
* Si l’EEPROM est vide → AP immédiat
* Si le WiFi échoue après 30 secondes → AP fallback

## Paramètres AP :

* SSID : ESP32-Setup
* Mot de passe : 12345678
* IP : 192.168.4.1

## Etapes: 

### 4.1 — Lire les identifiants au boot

Définir les variables global
```cpp
String wifiSSID = "";
String wifiPASS = "";
```

Dans setup() :
```cpp
wifiSSID = readEEPROM(SSID_ADDR, 100);
wifiPASS = readEEPROM(PASS_ADDR, 100);
```
Puis :
```cpp
bool hasCredentials = wifiSSID.length() > 0 && wifiPASS.length() > 0;
```
Explication:
* Si les deux champs sont vides → pas de WiFi configuré.

### 4.2 — Fonction pour démarrer en Access Point

Ajoute :
```cpp
void startAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Setup", "12345678");

  Serial.println("Mode AP actif : ESP32-Setup / 12345678");
}
```
Explication: 
* Le mode AP permet à l’utilisateur de se connecter directement à l’ESP32.
* L’interface web reste accessible via 192.168.4.1.

### 4.3 — Tentative de connexion WiFi avec timeout

Ajoute :
```cpp
bool tryConnectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPASS.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 30000) {
    delay(300);
    Serial.print(".");
  }

  return WiFi.status() == WL_CONNECTED;
}
```
Explication:
* On attend 30 secondes maximum.
* Si la connexion échoue → fallback AP.

### 4.4 — Logique complète dans setup()

Ajoute:
```cpp
if (!hasCredentials) {
  startAccessPoint();
} else {
  if (!tryConnectWiFi()) {
    startAccessPoint();
  }
}
```
Explication:
* Si pas de WiFi → AP direct.
* Si WiFi configuré → tentative.
* Si échec → AP fallback.

# 5. 🔵🟠 Étape 4 — Utilisation de la LED intégrée

La LED permet d’indiquer visuellement l’état du système et de savoir immédiatement dans quel mode se trouve l’ESP32.

## Couleurs utilisées :

* Bleu → Mode Access Point
* Orange → Tentative de connexion WiFi
* OFF → WiFi connecté

## Pourquoi c’est utile ?

* Diagnostic immédiat
* Compréhension intuitive du statut
* Très pratique en atelier ou en installation

## Ce qu’il faut savoir

L’ESP32‑S3 DevKitC‑1 a une LED RGB adressable (WS2812 / NeoPixel) soudée directement sur la carte.
* La LED RGB intégrée est un WS2812 (NeoPixel).
* Elle est connectée sur le GPIO 48.
* Elle nécessite un driver spécial (FastLED ou Adafruit NeoPixel).
* Elle permet d’afficher des couleurs, parfait pour indiquer les états :
  * Bleu → Access Point
  * Orange → Tentative WiFi
  * Off → WiFi connecté

## Etapes: 

### 5.1. Ajouter la librairie NeoPixel

Dans Arduino IDE installer la librairie: Adafruit NeoPixel

Puis ajouter en haut du code :
```cpp
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define LED_COUNT 1

Adafruit_NeoPixel rgbLED(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
```

### 5.2. Initialiser la LED dans setup()

```cpp
rgbLED.begin();
rgbLED.setBrightness(50);  // luminosité (0–255)
ledOff();             // éteint la LED
```
### 5.3. Fonctions utilitaires pour changer la couleur

Ajoute ces fonctions :
```cpp
void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  rgbLED.setPixelColor(0, rgbLED.Color(r, g, b));
  rgbLED.show();
}

void ledBlue() {
  setLEDColor(0, 0, 255);
}

void ledOrange() {
  setLEDColor(255, 80, 0);
}

void ledOff() {
  setLEDColor(0, 0, 0);
}
```

### 5.4. Intégration dans les étapes du tutoriel

🔵 Access Point → LED BLEUE: 

Dans startAccessPoint() :
```cpp
ledBlue();
```

🟠 Tentative WiFi → LED ORANGE

Dans tryConnectWiFi() :
```cpp
ledOrange();
```

⚫ WiFi connecté → LED OFF

Toujours dans tryConnectWiFi() :
```cpp
if (WiFi.status() == WL_CONNECTED) {
    ledOff();
    return true;
}
```

🔵 Fallback WiFi → LED BLEUE

Si la connexion échoue :
```cpp
ledBlue();
startAccessPoint();
```

# 6. 🧩 Étape 5 — Intégration complète dans le code final

À cette étape, toutes les fonctionnalités sont fusionnées :
* Lecture EEPROM
* Tentative WiFi
* Fallback AP
* LED indicatrice
* API /setWifi
* API /playNote
* Serveur Web
* mDNS
* LittleFS

Le code final est désormais un mini‑serveur MIDI configurable, autonome et robuste.

# 7. 🧪 Section de test — Vérifier que tout fonctionne

Voici les tests recommandés pour valider le système :

## Test 1 — Boot sans identifiants

1. Effacer l’EEPROM
2. Redémarrer l’ESP32
3. Vérifier :
  * LED bleue
  * AP ESP32-Setup
  * Interface accessible via 192.168.4.1

## Test 2 — Enregistrement du WiFi

1. Appeler :
```bash
http://192.168.4.1/setWifi?ssid=MonWifi&password=MonMotDePasse
```
2. Redémarrer l’ESP32
3. Vérifier :
  * LED orange pendant la tentative
  * LED OFF si connexion OK
 * LED bleue si échec

## Test 3 — API MIDI

1. Appeler :
```bash
http://midiserver.local/playNote?note=60&channel=1
```
2. Vérifier que la note est jouée

## Test 4 — Fallback WiFi

1. Entrer un mauvais mot de passe
2. Redémarrer
3. Vérifier :
  * LED orange pendant 30 s
  * LED bleue ensuite
  * AP actif

# 8. 📘 Conclusion

Ce tutoriel vous a permis de transformer un simple projet ESP32 en un système :
* Autonome
* Configurable
* Robuste
* Facile à diagnostiquer
* Accessible même en cas de panne WiFi

Vous disposez maintenant d’une base idéale pour :
* Un contrôleur MIDI autonome
* Un module IoT configurable
* Un système embarqué avec interface web
