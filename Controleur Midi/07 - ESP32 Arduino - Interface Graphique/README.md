# 🎹 Interface Graphique Web avancée pour le projet USB‑MIDI ESP32‑S3

## 🎯 Objectif du projet

L'objectif de ce projet est de développer une interface web moderne, légère et configurable permettant de piloter et configurer le contrôleur MIDI basé sur ESP32-S3.

L'interface doit être conçue de manière à fonctionner :
- Sur un serveur web embarqué sur ESP32-S3
- En local sur un PC pendant le développement.
- Avec exactement les mêmes fichiers HTML, CSS et JavaScript afin d'éviter les différences entre l'environnement de développement et le produit final.

# 📺 Vidéo

Lien Youtube: [https://www.youtube.com/@Baronnix/playlists](https://www.youtube.com/@Baronnix/playlists)

# 📦 Prérequis

* Carte ESP32‑S2 / ESP32‑S3 compatible TinyUSB (Dans ce tutoriel on utilise la carte ESP32‑S3 DevKit C1)
* Câble USB‑C
* Un ordinateur (tutoriel réalisé sur Windows)
* Arduino IDE installé
* esp32 installé dans le gestionnaire de cartes (esp32 par Espressif Systems - 3.3.10)

Ce tutoriel repart du code final du tutoriel [06 - ESP32 Arduino - AsyncWebServer or Webserver](https://github.com/Baronnix/ESP32/tree/main/Controleur%20Midi/06%20-%20ESP32%20Arduino%20-%20AsyncWebServer%20or%20Webserver)

# Dependances et Versions

* Arduino IDE - 2.3.10
* Arduino Plugins:
  * arduino-littlefs-upload - 1.6.3
* Cartes:
  * esp32 par Espressif Systems - 3.3.11
* Librairies
  * Adafruit NeoPixel - 1.15.5
  * Async TCP par ESP32Async - 3.5.0
  * ESPAsyncWebServer par ESP32Async - 3.12.1
  
# 🧩 Architecture souhaitée

L'ensemble des fichiers de l'interface devra être stocké dans le dossier `data/` afin de pouvoir être déployé directement dans LittleFS.

L'interface devra contenir:
* Un menu
* Une page permettant de changer le réseau Wifi auquel le contrôleur se connecte
* Un dashboard configurable permettant de:
    * changer la configuration des touches et notes
    * jouer les notes MIDI grâce aux touches

## Menu principal

Le menu doit être partagé entre toutes les pages.

Il sera chargé dynamiquement afin d'éviter la duplication du code dans les pages suivantes:
* Dashboard
* Configuration WiFi

Le menu devra être responsive (adaptable) pour une utilisation sur :
* PC
* Tablette
* Smartphone

## Dashboard

Objectif: Afficher des boutons configurables qui permettent de jouer les notes MIDI.

Contenu souhaité:
* Ajout/Supression de touches
* Taille et position des touches configurable
* Persistance de la configuration dans le browser
* Possibilité de récupérer en local et de ré-injecter la configuration

## Configuration WiFi

Objectif: Permettre à l'utilisateur de changer le réseau Wifi auquel se connecter

Contenu souhaité:
* Fonctionnalité listant les réseaux WIFI disponibles
* Définition de nom et mot de passe du réseau à utiliser
* Possibilité d'effacer le réseau WIFI prédédemment enregistré

Limitation:
* Une fois enregistré, il faut redémarrer le contrôleur pour qu'il se connecte au nouveau WIFI.

# 📁 Structure du projet

```text
data/
│
├── index.html
├── LICENSE bootstrap.txt
├── LICENSE gridstack.txt
│
├── css/
│   ├── bootstrap.min.css
│   └── globals.css
│   └── gridstack.min.css
│
├── html/
│   ├── dashboard.html
│   └── menu.html
│   └── wifiSettings.html
│
└── js/
    ├── bootstrap-autocomplete.min.js
    ├── bootstrap.bundle.min.js
    ├── bootstrap.min.js
    └── functions-gridstack.js
    └── functions.js
    └── gridstack-all.min.js
    └── jquery.min.js
```

# 🛠️ Librairies utilisées

## Bootstrap

* Documentation: https://getbootstrap.com/
* CDN links: https://www.jsdelivr.com/package/npm/bootstrap

## Bootstrap-autocomplete

* Documentation: https://bootstrap-autocomplete.readthedocs.io/en/latest/
* CDN links: https://www.jsdelivr.com/package/npm/bootstrap-autocomplete

## jquery

* Documentation: https://jquery.com/
* CDN links: https://www.jsdelivr.com/package/npm/jquery

## Gridstack

* Documentation: https://gridstackjs.com/
* CDN links: https://www.jsdelivr.com/package/npm/gridstack


# 📥 Développement local

Afin de pouvoir tester les pages web sur PC sans les envoyer systématiquement dans l'ESP32, un serveur HTTP local sera utilisé.

## Pourquoi ?

L'inclusion de fragments HTML par JavaScript utilise :

```JavaScript
fetch("fragments/menu.html")
```

Cette méthode ne fonctionne pas lorsqu'un fichier HTML est ouvert directement avec :

```Text
file:///C:/Projet/index.html
```
Le navigateur bloque alors les requêtes pour des raisons de sécurité (CORS).

Un serveur web local est donc nécessaire.

## Installation de Python sous Windows

1. Télécharger Python : https://www.python.org/downloads/
2. Lancer l'installation en acceptant d'ajouter python aux variables d'environnement: Add Python to PATH
3. Ouvrir un PowerShell
4. Démarrer un environment Python:
```bash
python
```
5. Pour quitter l' environment Python:
```python
exit()
```

## Démarrage du serveur local

1. Se placer dans le dossier data, exemple :
```BAT
cd C:\ProjetESP32\data
```
2. Lancer le server python: 
```BAT
python -m http.server 8000
```

## Accès à l'interface

1. Ouvrir dans le navigateur et aller à l'adresse:
    * http://localhost:8000

L'ensemble des pages HTML, CSS et JavaScript sera alors accessible exactement comme sur l'ESP32.

# Déploiement sur ESP32

Les mêmes fichiers seront stockés dans LittleFS.

Structure cible :

```Text
LittleFS
│
├── index.html
├── LICENSE bootstrap.txt
├── LICENSE gridstack.txt
├── css/
├── html/
├── je/
└── images/
```

Le code ESP32 devra simplement servir les fichiers du système de fichiers.

Exemple :
```cpp
server.on("/html/menu.html", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/html/menu.html");
    });
```

# Créer son propre contrôleur MIDI en utilisant ce code

1. Cloner le dépôt: https://github.com/Baronnix/ESP32.git
2. Ouvrir le dossier cloné
3. Aller dans Controleur Midi/07 - ESP32 Arduino - Interface Graphique/midi_REST_API_HTML/
4. Lancer le fichier .ino dans Arduino
5. Installer les librairies, plugins et cartes nécessaires dans Arduino (Voir turoriels précédents)
6. Configurer la carte dans Arduino IDE
    1. Choisir le type de carte:  Outils → Type de carte → ESP32S3 Dev Module
    2. Choisir le mode USB: Outils → USB Mode → USB OTG (TinyUSB)
    3. Choisir l'USB firmware: Outils → USB CDC On Boot → Disabled
    4. Choisir la taille de la mémoire Flash suivant votre ESP32
    5. Choisir Partition Schema: Default 4MB with spiffs
![config_carte](images/config_carte.jpg)
7. Branche l'ESP32S3 devkit sur le port USB-TTL
8. Redémarre l'ESP32S3 devkit en mode flashage (bouton BOOT maintenu appuyé + et appui court sur bouton RESET)
9. Choisit le bon port COM (série)
10. Compiler et flasher le code: Sketch → Upload
11. Flasher LittleFS
    1. Appuie sur Ctrl + Shift + P (Windows)
    2. Tape/cherche : Upload LittleFS to Pico/ESP8266/ESP32
    3. Clique sur : Upload LittleFS to Pico/ESP8266/ESP32
  ![upload_LittleFS](images/upload_LittleFS.jpg)
    4. Attends le message : Completed upload.
    5. Les fichiers sont maintenant dans la flash.
12. Débrancher l'ESP32S3 et le brancher sur le port USB-OTG

A ce stade le contrôleur MIDI est prêt et fonctionnel.

# Notice Utilisateur du Contrôleur MIDI USB avec ESP32-S3

La notice utilisateur est disponible ici: [userguide.txt](https://github.com/Baronnix/ESP32/blob/main/Controleur%20Midi/07%20-%20ESP32%20Arduino%20-%20Interface%20Graphique/userguide.txt)