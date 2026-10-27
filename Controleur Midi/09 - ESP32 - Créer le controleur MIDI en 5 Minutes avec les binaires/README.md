# 🚀 ESP32 / ESP32-S3 : Créer le contrôleur MIDI en 5 Minutes avec les binaires ! 

Dans ce tutoriel, nous allons apprendre à créer un contrôleur MIDI basé sur un ESP32-S3 en flashant les binaires développés dans les tutoriels précédents.

À la fin de ce guide, vous aurez un contrôleur MIDI connecté configurable pour moins de 15 euros sans avoir besoin de connsaissance de programmation.

# 📺 Vidéo

Lien Youtube: [https://www.youtube.com/watch?v=4ILaIuZSrds](https://www.youtube.com/watch?v=4ILaIuZSrds)

# 📦 Prérequis

* Carte ESP32‑S2 / ESP32‑S3 compatible TinyUSB (Dans ce tutoriel on utilise la carte ESP32‑S3 DevKit C1)
* Câble USB‑C
* Un ordinateur (tutoriel réalisé sur Windows)

# 1. Installation du driver CH340 (Optionnel et nécessaire seulement si cela n'a pas été installé précédemment)

Pour pouvoir flasher, la carte de l'ESP32-S3 doit être reconnu. Il faut installer le driver CH340 (Windows):

1. Télécharger le driver officiel: https://sparks.gogo.co.nz/ch340.html
2. Dézipper le fichier téléchargé
3. Lancer CH34x_Install_Windows_v3_4.EXE
4. Cliquer sur INSTALL

# 2. Récupérer les binaires

1. Aller sur [https://github.com/Baronnix/ESP32/tree/main/Controleur%20Midi/09%20-%20ESP32%20-%20Cr%C3%A9er%20le%20controleur%20MIDI%20en%205%20Minutes%20avec%20les%20binaires/build](https://github.com/Baronnix/ESP32/tree/main/Controleur%20Midi/09%20-%20ESP32%20-%20Cr%C3%A9er%20le%20controleur%20MIDI%20en%205%20Minutes%20avec%20les%20binaires/build)
2. Récupérez les fichiers du dossier build

# 3. Télécharger ESP Flash Download Tool

1. Aller sur le site Espressif: https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32/production_stage/tools/flash_download_tool.html
2. Télécharger: ESP Flash Download Tool
3. Décompresser puis lancer : flash_download_tool.exe

# 5. Configurer l'outil

1. Au démarrage, choisir :
    * ChipType : ESP32-S3
    * WorkMode: Develop
    * LoadMode: UART
2. Ajouter les binaires tel que défini dans le fichier "flash.txt":
    * bootloader.bin -> 0x0000
    * partitions.bin -> 0x8000
    * boot_app0.bin -> 0xe000
    * firmware.bin -> 0x10000
    * littlefs.bin -> 0x00290000
    
# 6. Flasher l'ESP32-S3

1. Branche l'ESP32S3 devkit sur le port USB-TTL
2. Mettre l'ESP32-S3 en mode téléchargement: 
    1. Maintenir BOOT
    2. Appuyer sur RESET
    3. Relâcher RESET
    4. Relâcher BOOT
3. Dans ESP Flash Download Tool choisir le bon port COM
4. Appuyer sur START pour démarrer le flashage
Les logs doivent afficher :
```text
Sync...
Downloading...
Finish
```

✅ Le contrôleur MIDI est terminé, branchez le sur le port USB-OTG pour l'utiliser

La notice utilisateur est disponible ici: [userguide.txt](https://github.com/Baronnix/ESP32/blob/main/Controleur%20Midi/09%20-%20ESP32%20-%20Cr%C3%A9er%20le%20controleur%20MIDI%20en%205%20Minutes%20avec%20les%20binaires/userguide.txt)