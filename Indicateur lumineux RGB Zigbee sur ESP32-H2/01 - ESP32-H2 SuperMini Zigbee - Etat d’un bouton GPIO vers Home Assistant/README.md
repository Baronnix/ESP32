# ESP32-H2 SuperMini Zigbee : état d’un bouton GPIO vers Home Assistant

Ce tutoriel montre comment créer un composant Zigbee avec un **ESP32-H2 SuperMini**. Un bouton externe raccordé à **GPIO14** est surveillé et son état (appuyé ou relâché) est envoyé à chaque changement, via une passerelle/coordinatrice Zigbee utilisant **Zigbee2MQTT**, vers Home Assistant.

> [!IMPORTANT]
> L'ESP32-H2 ne publie pas directement en MQTT. Il communique en **Zigbee** avec la coordinatrice. C'est **Zigbee2MQTT** qui convertit ensuite les messages Zigbee en messages MQTT pour Home Assistant.


# 📺 Vidéo

Lien Youtube: [https://www.youtube.com/@Baronnix](https://www.youtube.com/@Baronnix)

## 1. Architecture

```text
ESP32-H2 SuperMini
   │
   │ Zigbee 3.0 / IEEE 802.15.4
   ▼
Coordinateur Zigbee
   │
   │ Zigbee2MQTT
   ▼
Broker MQTT
   │
   ▼
Home Assistant
```

## 2. Fonctionnement attendu

Le firmware gère deux modes :

1. **Mode appairage**
   - L'ESP32-H2 cherche un réseau Zigbee ouvert.
   - Zigbee2MQTT doit avoir l'option **Appairage autorisé** activée.
   - Une fois appairé, le composant apparaît dans Zigbee2MQTT et Home Assistant.

2. **Mode fonctionnement**
   - Un bouton externe est lu sur GPIO14 avec une résistance de tirage interne.
   - Son état est exposé avec un cluster Zigbee **Binary Input** et envoyé à chaque changement stable (avec anti-rebond).
   - L’état courant est également envoyé une fois après la connexion au réseau Zigbee.
   - Un appui long sur le bouton `BOOT` réinitialise la configuration Zigbee et relance l'appairage.

> Le bouton est câblé entre GPIO14 et GND : `LOW` signifie **appuyé** et `HIGH` signifie **relâché**. Le bouton `BOOT` reste réservé à la réinitialisation Zigbee.

## 3. Matériel nécessaire

- Une carte **ESP32-H2 SuperMini** avec connecteur USB-C et 4 Mo de flash

![ESP32-H2_SuperMini](../images/ESP32-H2_SuperMini.jpg)

- Un bouton poussoir externe, raccordé entre GPIO14 et GND

![bouton_poussoir](../images/bouton_poussoir.jpg)

- Un câble USB adapté à la carte
- Home Assistant avec une passerelle Zigbee
  Voir tutoriels:
    * [02 - Installation Home Assistant](https://www.youtube.com/watch?v=F-s3jXZxmUI)
    * [04 - Zigbee Installation et Utilisation](https://www.youtube.com/watch?v=F-s3jXZxmUI))
- Accès à l’interface Home Assistant 
- Arduino IDE 2.x avec le core Espressif ESP32 (Voir tutoriel [01 - ESP32 Arduino - USB‑C MIDI Controleur - Premiere Programmation](https://www.youtube.com/watch?v=1if2j6FiMQ4))

## 3.1 Broches utiles et cablage de l’ESP32-H2 SuperMini

| Fonction | Broche | Remarque |
|---|---:|---|
| Bouton BOOT | GPIO9 | Actif à l’état bas, également broche de strapping |
| LED intégrée | GPIO13 sur de nombreuses variantes | Inutile dans ce tutoriel |
| USB-C natif | USB Serial/JTAG | Utilisé pour le flash et le moniteur série |
| Bouton externe | GPIO14 | Câbler le bouton entre GPIO14 et GND |

> Les cartes SuperMini sont produites par plusieurs fabricants. Vérifier le marquage de votre exemplaire avant le câblage. GPIO9 est réservé au bouton BOOT ; le bouton externe utilise GPIO14.

![cablage](images/cablage.png)

## 4. Préparation d'Arduino IDE

### 4.1 Installer le support ESP32

Dans **Arduino IDE > File > Preferences**, ajouter l'URL suivante dans **Additional Boards Manager URLs** :

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

Puis ouvrir **Tools > Board > Boards Manager**, rechercher `esp32` et installer le package **esp32 by Espressif Systems**.

### 4.2 Sélectionner les options Zigbee

Dans le menu **Tools** :

- **Board** : `ESP32H2 Dev Module`
- **USB CDC On Boot** : `Enabled`
- **Upload Mode** : `UART0 / Hardware CDC` si cette option est proposée
- **Zigbee mode** : `Zigbee ED (end device)`
- **Partition Scheme** : `Zigbee 4MB with spiffs`
- **Erase All Flash Before Sketch Upload** : `Enabled` pour le premier flash ou après un problème d'appairage
- **Core Debug Level** : `Info` ou `Verbose` pendant les tests

> Les noms des menus peuvent légèrement varier selon la version du core Arduino ESP32.

## 5. Code Arduino

Créer un nouveau sketch, par exemple `esp32_h2_button_zigbee.ino`, puis copier le code suivant :

```cpp
#include <Arduino.h>
#include "Zigbee.h"

// Endpoint Zigbee du capteur binaire.
#define SENSOR_ENDPOINT 10

// Durée d'appui permettant un factory reset Zigbee.
static const uint32_t FACTORY_RESET_PRESS_MS = 5000;

// Sur l’ESP32-H2 SuperMini, le bouton BOOT est généralement relié à GPIO9.
static const uint8_t BOOT_BUTTON_PIN = 9;
static const uint8_t EXTERNAL_BUTTON_PIN = 14;
static const uint32_t DEBOUNCE_MS = 50;
static const uint32_t REPORT_RETRY_MS = 1000;

ZigbeeBinary buttonSensor(SENSOR_ENDPOINT);

uint32_t bootButtonPressedAt = 0;
uint32_t lastDebounceTime = 0;
uint32_t lastReportAttempt = 0;
bool lastRawButtonState = false;
bool stableButtonState = false;
bool reportPending = true;
bool resetTriggered = false;

void handleFactoryResetButton() {
  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    if (bootButtonPressedAt == 0) {
      bootButtonPressedAt = millis();
      resetTriggered = false;
    }

    if (!resetTriggered &&
        millis() - bootButtonPressedAt >= FACTORY_RESET_PRESS_MS) {
      resetTriggered = true;
      Serial.println("Réinitialisation Zigbee et redémarrage...");
      delay(200);
      Zigbee.factoryReset();
    }
  } else {
    bootButtonPressedAt = 0;
    resetTriggered = false;
  }
}

void handleExternalButton() {
  const bool rawButtonState = digitalRead(EXTERNAL_BUTTON_PIN) == LOW;
  if (rawButtonState != lastRawButtonState) {
    lastRawButtonState = rawButtonState;
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime >= DEBOUNCE_MS &&
      rawButtonState != stableButtonState) {
    stableButtonState = rawButtonState;
    reportPending = true;
    Serial.printf("Bouton GPIO14 : %s\n", stableButtonState ? "appuyé" : "relâché");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(EXTERNAL_BUTTON_PIN, INPUT_PULLUP);

  lastRawButtonState = digitalRead(EXTERNAL_BUTTON_PIN) == LOW;
  stableButtonState = lastRawButtonState;

  // Configuration de l'endpoint avant son ajout à la pile Zigbee.
  buttonSensor.setManufacturerAndModel("DIY", "ESP32-H2-GPIO14-Button");
  if (!buttonSensor.addBinaryInput() ||
      !buttonSensor.setBinaryInputApplication(BINARY_INPUT_APPLICATION_TYPE_HVAC_OTHER) ||
      !buttonSensor.setBinaryInputDescription("Bouton GPIO14") ||
      !buttonSensor.setBinaryInput(stableButtonState)) {
    Serial.println("Erreur : impossible de configurer l'entrée binaire Zigbee.");
    ESP.restart();
  }

  Zigbee.addEndpoint(&buttonSensor);

  Serial.println("Démarrage du composant Zigbee...");
  Serial.println("Si le composant n'est pas encore associé, activez Permit join dans Zigbee2MQTT.");

  if (!Zigbee.begin()) {
    Serial.println("Erreur : impossible de démarrer la pile Zigbee.");
    delay(1000);
    ESP.restart();
  }

  Serial.println("Pile Zigbee démarrée.");
}

void loop() {
  handleFactoryResetButton();
  handleExternalButton();

  if (!Zigbee.connected()) {
    // Mode 1 : appairage ou reconnexion au réseau Zigbee.
    static uint32_t lastPairingMessage = 0;
    if (millis() - lastPairingMessage >= 5000) {
      lastPairingMessage = millis();
      Serial.println("Mode appairage : attente de connexion au réseau Zigbee...");
    }
    delay(20);
    return;
  }

  // Publie aussi l'état initial et réessaie si le rapport Zigbee échoue.
  if (reportPending && millis() - lastReportAttempt >= REPORT_RETRY_MS) {
    lastReportAttempt = millis();
    if (buttonSensor.setBinaryInput(stableButtonState) &&
        buttonSensor.reportBinaryInput()) {
      Serial.printf("État du bouton envoyé : %s\n", stableButtonState ? "appuyé" : "relâché");
      reportPending = false;
    } else {
      Serial.println("Erreur : échec de l'envoi du rapport Zigbee, nouvel essai prévu.");
    }
  }

  delay(20);
}
```

## 6. Compilation et flash

1. Connecter l'ESP32-H2 SuperMini avec un câble USB-C prenant en charge les données.
2. Sélectionner `ESP32H2 Dev Module`, puis le port série dans **Tools > Port**.
3. Si le port n’apparaît pas, maintenir `BOOT`, appuyer brièvement sur `RST`, relâcher `RST`, puis relâcher `BOOT`.
4. Cliquer sur **Upload**.
5. Ouvrir le **Serial Monitor** à `115200 bauds`.
6. Vérifier que le message suivant apparaît :

```text
Mode appairage : attente de connexion au réseau Zigbee...
```

## 7. Appairage avec Zigbee2MQTT

1. Ouvrir l'interface Zigbee2MQTT.
2. Cliquer sur **Appairage autorisé**.
3. Redémarrer l'ESP32-H2, ou maintenir `BOOT` pendant environ cinq secondes pour effacer l'ancien réseau.
4. Attendre la fin de l'interview Zigbee.
5. Renommer le composant, par exemple `esp32_h2_button`.

Une fois connecté, l'état courant du bouton est envoyé. À chaque appui ou relâchement, le moniteur série doit afficher des lignes similaires à :

```text
Bouton GPIO14 : appuyé
État du bouton envoyé : appuyé
Bouton GPIO14 : relâché
État du bouton envoyé : relâché
```

## 8. Affichage dans Home Assistant

Si votre version de Zigbee2MQTT prend en charge le cluster **Binary Input** et que la découverte MQTT est activée, une entité peut être créée automatiquement dans Home Assistant. Sinon, configurez un external converter Zigbee2MQTT pour exposer l'attribut binaire.

Pour l'ajouter au tableau de bord :

1. Ouvrir **Settings > Devices & services > MQTT**.
2. Rechercher le composant `ESP32-H2-GPIO14-Button`.
3. Vérifier que l'état du bouton change lors d'un appui et d'un relâchement.
4. Ouvrir un tableau de bord.
5. Cliquer sur **Edit dashboard > Add card**.
6. Choisir une carte **Tile** ou **Entities** adaptée à une entité binaire.
7. Sélectionner l'entité créée par Zigbee2MQTT.

Exemple de carte Lovelace :

```yaml
type: tile
entity: binary_sensor.esp32_h2_button
name: Bouton GPIO14
```

Adapter `entity` au nom réellement créé dans Home Assistant. Selon le support du cluster **Binary Input** par la version de Zigbee2MQTT utilisée, l'entité peut ne pas être découverte automatiquement ; voir le dépannage ci-dessous.

## 9. Vérification côté MQTT

Pour vérifier le trajet complet, écouter le topic du composant :

```bash
mosquitto_sub -h ADDRESS_MQTT -u MQTT_USER -P MQTT_PASSWORD \
  -t 'zigbee2mqtt/esp32_h2_button' -v
```

Exemple de message attendu :

```json
{
  "linkquality": 128,
  "presentValue": true
}
```

Le nom et le format de la propriété dépendent de la version de Zigbee2MQTT et du convertisseur associé au cluster **Binary Input**.

## 10. Dépannage

### Le composant ne rejoint pas le réseau

- Vérifier que **Permit join** est actif dans Zigbee2MQTT.
- Rapprocher l'ESP32-H2 de la coordinatrice pour le premier appairage.
- Activer **Erase All Flash Before Sketch Upload**, reflasher, puis recommencer.
- Maintenir `BOOT` pendant environ cinq secondes pour appeler `Zigbee.factoryReset()`.
- Vérifier que **Zigbee mode** est configuré sur `Zigbee ED`.
- Vérifier que le schéma de partition Zigbee est sélectionné.
- Si le téléchargement reste bloqué, utiliser la séquence `BOOT` + `RST` décrite dans la section de flash.
- Utiliser un câble USB-C de données, certains câbles ne fournissent que l’alimentation.

### Le composant est détecté mais marqué comme non pris en charge

Le réseau Zigbee peut reconnaître le composant sans que Zigbee2MQTT possède un convertisseur correspondant à sa signature fabricant/modèle. Dans ce cas :

1. Vérifier si l'attribut `presentValue` du cluster **Binary Input** est tout de même visible dans les informations du composant.
2. Relancer l'interview du composant dans Zigbee2MQTT.
3. Supprimer le composant, effectuer un factory reset, puis l'appairer de nouveau.
4. Si nécessaire, créer un **external converter** Zigbee2MQTT pour le modèle `ESP32-H2-GPIO14-Button`.
5. Vérifier la prise en charge du cluster **Binary Input** par l'intégration Zigbee utilisée avant d'attendre une entité Home Assistant.

### L'état du bouton n'est pas publié

- Vérifier que le moniteur série affiche `Bouton GPIO14` lors d'un changement d'état, puis `État du bouton envoyé`.
- Vérifier que `Zigbee.connected()` reste vrai.
- Regarder les logs Zigbee2MQTT et le topic MQTT du composant.
- Vérifier le câblage entre GPIO14 et GND ; le bouton est actif à l'état bas.
- Si aucune entité n'est créée, vérifier la prise en charge du cluster **Binary Input** par Zigbee2MQTT ou utiliser un external converter adapté.
- Désactiver temporairement les modes d'économie d'énergie pendant le diagnostic.

### Erreur de compilation sur une méthode Zigbee

L'API Zigbee d'Arduino ESP32 évolue. Vérifier que le core **esp32 by Espressif Systems** est à jour. Si les méthodes `ZigbeeBinary` utilisées dans ce tutoriel ne sont pas disponibles, ouvrir l'exemple fourni avec le core :

```text
File > Examples > Zigbee > Zigbee_Binary_Input_Output
```

Puis adapter le tutoriel à la signature des méthodes de cette version.

## 11. Limites et améliorations possibles

- L'état est transporté avec le cluster **Binary Input** ; son exposition dans Home Assistant dépend de la prise en charge de ce cluster par Zigbee2MQTT.
- L'anti-rebond logiciel de 50 ms évite de publier plusieurs changements dus aux rebonds mécaniques du bouton.
- Pour une alimentation sur batterie, ajouter le deep sleep seulement après validation de l'appairage et du reporting.
- Après toute modification importante des endpoints ou clusters, faire un factory reset et réappairer le composant afin de relancer une interview complète.

## 12. Critères de réussite

Le tutoriel est validé lorsque :

- L'ESP32-H2 apparaît dans Zigbee2MQTT.
- L'appairage survit à un redémarrage de la carte.
- L'état appuyé/relâché du bouton GPIO14 est envoyé à chaque changement.
- L'état est visible dans les logs Zigbee2MQTT.
- L'état est publié sur un topic MQTT et visible dans Home Assistant (si le cluster est pris en charge).
- Une entité correspondante est visible dans Home Assistant.
- Un appui long sur `BOOT` efface le réseau Zigbee et permet un nouvel appairage.
