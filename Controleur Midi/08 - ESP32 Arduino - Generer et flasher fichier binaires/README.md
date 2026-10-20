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
![config_carte](images/config_carte.jpg)

⚠️ Le schéma de partition choisi est important. Il devra rester identique lors du flashage (Par exemple : Default 4MB with spiffs).

## Compiler

1. Aller dans: Croquis → Vérifier/Compiler
2. Une fois terminé aller dans: Croquis → Export Compiled Binary

Arduino crée alors plusieurs fichiers binaires.

Sous Windows, ils se trouvent généralement dans :

Plain Text
C:\Users\TON_USER\AppData\Local\Temp\arduino\sketches\XXXXX\

ou dans le dossier du projet selon la version de l'IDE.

Tu trouveras quelque chose comme :

Plain Text
MonProjet.ino.bin
MonProjet.ino.bootloader.bin
MonProjet.ino.partitions.bin

Conserve ces fichiers.

2. Générer le binaire LittleFS

Pour cela il faut utiliser l'outil LittleFS Upload.

Selon ta version Arduino IDE :

Arduino IDE 2.x

Installe le plugin :

Plain Text
ESP32 LittleFS Upload

ou utilise l'outil CLI correspondant.

Génération du fichier LittleFS

Le dossier de ton projet doit ressembler à :

Plain Text
MonProjet/
 
├── MonProjet.ino
└── data/
├── index.html
├── style.css
├── config.json
└── ...

Lancer :

Plain Text
Tools → ESP32 Sketch Data Upload

ou

Plain Text
Upload LittleFS to Pico/ESP32

(selon l'extension installée)

Cette opération crée une image filesystem.

On récupère ensuite un fichier similaire à :

Plain Text
littlefs.bin

ou

Plain Text
spiffs.bin

selon le plugin.

3. Récupérer les adresses de flash

Très important.

Le plus simple :

dans le dossier de compilation, ouvrir :

Plain Text
partitions.csv

ou connaître le schéma choisi.

Pour la plupart des ESP32-S3 avec LittleFS :

Élément	AdresseBootloader	0x0000
Partition Table	0x8000
Application	0x10000
LittleFS	dépend du partitionnement
Trouver l'adresse exacte LittleFS

Dans les logs de l'upload LittleFS on voit souvent :

Plain Text
Uploading LittleFS ...
Start address : 0x670000

ou similaire.

Autre méthode :

regarder le CSV de partition :

Exemple :

CSV
nvs,data,nvs,0x9000,0x5000
otadata,data,ota,0xe000,0x2000
app0,app,ota_0,0x10000,0x330000
littlefs,data,spiffs,0x340000,0x4C0000

Ici LittleFS commence à :

Plain Text
0x340000

C'est cette adresse qu'il faudra utiliser.

4. Télécharger ESP Flash Download Tool

Télécharger :

Plain Text
ESP Flash Download Tool

depuis le site Espressif.

Décompresser puis lancer :

Plain Text
flash_download_tool.exe
5. Configurer l'outil

Au démarrage :

Choisir :

Plain Text
ChipType : ESP32-S3

Puis :

Plain Text
Develop
6. Ajouter les binaires

Dans la fenêtre principale :

Ajouter les fichiers suivants :

Bootloader
Plain Text
MonProjet.ino.bootloader.bin

Adresse :

Plain Text
0x0000
Partition Table
Plain Text
MonProjet.ino.partitions.bin

Adresse :

Plain Text
0x8000
Firmware
Plain Text
MonProjet.ino.bin

Adresse :

Plain Text
0x10000
LittleFS
Plain Text
littlefs.bin

Adresse :

Plain Text
adresse du partitionnement LittleFS

Exemple :

Plain Text
0x340000

Configuration typique :

Plain Text
☑ bootloader.bin 0x0000
☑ partitions.bin 0x8000
☑ MonProjet.bin 0x10000
☑ littlefs.bin 0x340000
7. Paramètres Flash

Généralement :

Plain Text
SPI SPEED : 80 MHz

ou

Plain Text
40 MHz
Plain Text
SPI MODE : DIO

ou

Plain Text
QIO

reprendre exactement la configuration Arduino.

Plain Text
FLASH SIZE :

Exemples :

Plain Text
4MB
8MB
16MB

Choisir la taille réelle de la carte.

8. Mettre l'ESP32-S3 en mode téléchargement

Certaines cartes :

Plain Text
BOOT + RESET

Procédure :

Maintenir BOOT
Appuyer sur RESET
Relâcher RESET
Relâcher BOOT

D'autres cartes passent automatiquement en mode téléchargement.

9. Flashage

Dans ESP Flash Download Tool :

Choisir :

Plain Text
COMxx

Puis :

Plain Text
START

Le log doit afficher :

Plain Text
Sync...
Downloading...
Finish
10. Vérification

Ouvrir le moniteur série :

Plain Text
115200 bauds

Tu dois voir :

Plain Text
WiFi démarré
LittleFS mounted
MIDI ready

(ou les messages propres à ton projet)

11. Sauvegarder une "release" complète

Je te conseille de créer un dossier :

Plain Text
Release/
│
├── bootloader.bin
├── partitions.bin
├── firmware.bin
├── littlefs.bin
├── flash.txt

avec un fichier flash.txt :

Plain Text
ESP32-S3
 
bootloader.bin -> 0x0000
partitions.bin -> 0x8000
firmware.bin -> 0x10000
littlefs.bin -> 0x340000
 
Flash size : 16MB
Flash mode : QIO
Flash speed : 80MHz

Ainsi tu pourras reprogrammer n'importe quel ESP32-S3 sans Arduino IDE, uniquement avec ESP Flash Download Tool.

Si tu me fournis ton fichier .ino et ton schéma de partition (ou le dossier complet du projet), je peux t'indiquer les adresses exactes de flash, notamment celle de LittleFS, afin d'éviter toute erreur.

Fournissez vos commentaires sur BizChat
Pour little fs j'utilise arduino-littlefs-upload, y a t’il un moyen de trouver le binaire

Oui, mais cela dépend de la version du plugin arduino-littlefs-upload que tu utilises.

Le plugin génère forcément une image LittleFS avant de la téléverser. Il existe plusieurs méthodes pour la récupérer.

Méthode 1 : regarder la console Arduino (la plus simple)

Lance :

Plain Text
Outils → Upload LittleFS to ESP32

Puis ouvre la fenêtre de sortie détaillée.

Souvent, tu verras une commande du type :

Plain Text
mklittlefs.exe -c data -p 256 -b 4096 -s 0xBC0000 littlefs.bin

ou

Plain Text
Creating LittleFS image...
C:\Users\xxxx\AppData\Local\Temp\littlefs.bin

Le chemin du fichier généré apparaît parfois dans les logs.

Méthode 2 : rechercher le fichier après l'upload

Sous Windows, juste après un upload LittleFS réussi, cherche les fichiers récents :

Plain Text
*.bin

dans :

Plain Text
C:\Users\<ton_user>\AppData\Local\Temp

ou :

Plain Text
C:\Users\<ton_user>\AppData\Local\Arduino15

Trie par date de modification.

Tu trouveras souvent :

Plain Text
littlefs.bin

créé quelques secondes auparavant.

Méthode 3 : générer toi-même le binaire LittleFS

C'est la méthode que je préfère pour une release.

Dans le package ESP32, tu as normalement :

Plain Text
mklittlefs.exe

quelque part sous :

Plain Text
C:\Users\<ton_user>\AppData\Local\Arduino15\packages\esp32\

Exemple :

Plain Text
...\tools\mklittlefs\

Tu peux créer manuellement l'image :

BAT
mklittlefs.exe -c data -b 4096 -p 256 -s 0xBC0000 littlefs.bin

où :

data = ton dossier LittleFS
0xBC0000 = taille de la partition LittleFS

Cela te produit directement :

Plain Text
littlefs.bin

à conserver avec tes autres binaires.

Méthode 4 : récupérer les paramètres exacts utilisés par Arduino

Active :

Plain Text
Fichier → Préférences → Afficher les sorties détaillées

pour :

Plain Text
Compilation
Téléversement

Ensuite exécute :

Plain Text
Upload LittleFS

Le log complet affichera la commande exacte utilisée :

Plain Text
mklittlefs ...
esptool ...

Tu pourras alors reproduire exactement le même fichier binaire.

Encore mieux : utiliser le binaire déjà flashé

Si ton ESP32-S3 contient déjà le bon contenu LittleFS, tu peux l'extraire avec esptool :

BAT
esptool.py read_flash 0x670000 0x190000 littlefs.bin

où :

0x670000 = adresse de début de la partition LittleFS
0x190000 = taille de la partition

Tu récupères alors exactement l'image LittleFS présente dans la flash, prête à être réutilisée dans ESP Flash Download Tool.

C'est souvent la solution la plus fiable quand on veut fabriquer un pack de production (bootloader.bin + partitions.bin + firmware.bin + littlefs.bin).

Si tu me donnes :

la taille de flash de ton ESP32-S3 (8 Mo, 16 Mo ?),
ton schéma de partition (partitions.csv ou capture Arduino),

je peux te calculer exactement l'adresse LittleFS, sa taille, et la commande esptool read_flash à utiliser.

Fournissez vos commentaires sur BizChat