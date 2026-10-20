# 🚀 ESP32 / ESP32-S3 : Générer les BINAIRES et Flasher en 5 Minutes ! 

Dans ce tutoriel, nous allons apprendre à générer tous les fichiers binaires nécessaires à la distribution d'un projet ESP32 / ESP32-S3 développé avec l'IDE Arduino. Nous verrons comment récupérer le firmware compilé, créer l'image LittleFS, identifier les adresses de flash appropriées puis utiliser ESP Flash Download Tool pour programmer une carte ESP32 sans avoir besoin du code source ou de l'environnement Arduino.

À la fin de ce guide, vous serez capable de créer un véritable pack de déploiement composé des fichiers firmware.bin, bootloader.bin, partitions.bin et littlefs.bin, puis de reflasher rapidement un ESP32-S3 sur une nouvelle carte ou de partager votre projet sous forme de binaires prêts à l'emploi.

✅ Générer les binaires de votre projet Arduino
✅ Créer l'image LittleFS à partir du dossier data
✅ Comprendre les adresses de flash utilisées par l'ESP32
✅ Utiliser ESP Flash Download Tool comme un professionnel
✅ Déployer votre projet sur plusieurs cartes sans recompiler le code
✅ Distribuer votre firmware sans partager les sources Arduino

🚀 Une méthode particulièrement utile pour la production, les sauvegardes de projets, les tests sur plusieurs cartes ou le partage de vos créations ESP32 avec d'autres utilisateurs.

# 📺 Vidéo

Lien Youtube: [https://www.youtube.com/@Baronnix/playlists](https://www.youtube.com/@Baronnix/playlists)

# 📦 Prérequis

* Carte ESP32‑S2 / ESP32‑S3 compatible TinyUSB (Dans ce tutoriel on utilise la carte ESP32‑S3 DevKit C1)
* Câble USB‑C
* Un ordinateur (tutoriel réalisé sur Windows)
* Arduino IDE installé
* esp32 installé dans le gestionnaire de cartes (esp32 par Espressif Systems - 3.3.10)

Ce tutoriel repart du code final du tutoriel [07 - ESP32 Arduino - Interface Graphique](https://github.com/Baronnix/ESP32/tree/main/Controleur%20Midi/07%20-%20ESP32%20Arduino%20-%20Interface%20Graphique)

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

# 1. Compiler le firmware Arduino

## Ouvrir le projet

1. Ouvrir le projet dans Arduino IDE (Ouvre ton fichier .ino)
2. Configurer la carte dans Arduino IDE
    1. Choisir le type de carte:  Outils → Type de carte → ESP32S3 Dev Module
    2. Choisir le mode USB: Outils → USB Mode → USB OTG (TinyUSB)
    3. Choisir l'USB firmware: Outils → USB CDC On Boot → Disabled
    4. Choisir la taille de la mémoire Flash suivant votre ESP32
    5. Choisir Partition Schema: Default 4MB with spiffs

⚠️ Le schéma de partition choisi est important. Il devra rester identique lors du flashage (Par exemple : Default 4MB with spiffs).

## Compiler

1. Aller dans: Croquis → Vérifier/Compiler
2. Une fois terminé aller dans: Croquis → Export Compiled Binary

