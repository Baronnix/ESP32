
# Contrôler la luminosité d'une ampoule Zigbee avec un fader et un ESP32-H2

## Présentation du projet

Dans ce tutoriel, nous allons créer un variateur de luminosité physique à l'aide d'un fader (potentiomètre linéaire) HW-233 et d'un ESP32-H2 Super Mini.

L'objectif est de déplacer le curseur du fader pour faire varier en temps réel la luminosité d'une ampoule Zigbee pilotée par Home Assistant.

Le fonctionnement est le suivant :

1. Le fader fournit une tension analogique en fonction de la position de son curseur.
2. L'ESP32-H2 mesure cette tension sur une entrée analogique.
3. Le programme convertit la mesure en un pourcentage compris entre 0 et 100 %.
4. L'ESP32-H2 transmet cette valeur via Zigbee.
5. Zigbee2MQTT récupère la valeur et la transmet à Home Assistant.
6. Une automatisation Home Assistant adapte la luminosité de l'ampoule.

## 1. Matériel nécessaire

- Un ESP32-H2 Super Mini.
- Un module fader linéaire HW-233.
- Une ampoule Zigbee compatible avec Home Assistant.
- Un coordinateur Zigbee compatible Zigbee2MQTT.
- Quelques fils de connexion.

## 2. Branchement du fader HW-233

Le module HW-233 dispose de trois connexions utiles :

| Module HW-233 | ESP32-H2 Super Mini |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SIG | GPIO4 |

**Attention :** le signal analogique doit rester compatible avec la tension maximale acceptée par l'entrée de l'ESP32-H2. Dans ce montage, nous alimentons le module en 3,3 V. Ne branchez pas le module en 5 V.

## 3. Préparation de l'environnement Arduino IDE

Avant de commencer, vérifiez que :

- Le support ESP32 est installé dans Arduino IDE.
- La carte ESP32-H2 est sélectionnée.
- La bibliothèque Zigbee fournie avec le support ESP32 est disponible.
- Le port série correspondant à la carte est sélectionné.

Le programme de calibration et le programme Zigbee final sont deux sketches distincts.

## 4. Calibration du fader

### Pourquoi calibrer le fader ?

La conversion analogique de l'ESP32-H2 ne donne pas nécessairement une valeur allant exactement de 0 à 4095 sur toute la course mécanique du fader.

Il est donc préférable de mesurer les deux valeurs réellement obtenues :

- La valeur ADC lorsque le curseur est en butée basse.
- La valeur ADC lorsque le curseur est en butée haute.

Ces deux valeurs seront ensuite utilisées dans le programme Zigbee final.

### Sketch de calibration

Créez un nouveau sketch Arduino nommé `calibration_fader.ino` et téléversez le code suivant :

```cpp
#include <Arduino.h>

#define POTENTIOMETER_PIN 4
#define ADC_SAMPLES 64

int lireADC() {
  uint32_t total = 0;

  for (int i = 0; i < ADC_SAMPLES; i++) {
    total += analogRead(POTENTIOMETER_PIN);
    delay(2);
  }

  return total / ADC_SAMPLES;
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  analogReadResolution(12);

  Serial.println();
  Serial.println("=== CALIBRATION DU FADER ===");
  Serial.println("Placer le curseur en butee basse puis envoyer : m");
  Serial.println("Placer le curseur en butee haute puis envoyer : M");
}

void loop() {
  static uint32_t lastPrint = 0;

  if (millis() - lastPrint >= 400) {
    lastPrint = millis();
    Serial.printf("Valeur ADC : %d\n", lireADC());
  }

  while (Serial.available()) {
    char command = Serial.read();

    if (command == 'm') {
      int rawMin = lireADC();
      Serial.printf("ADC_RAW_MIN = %d\n", rawMin);
    }

    if (command == 'M') {
      int rawMax = lireADC();
      Serial.printf("ADC_RAW_MAX = %d\n", rawMax);
    }
  }
}
```

### Procédure de calibration

1. Téléversez le sketch de calibration sur l'ESP32-H2.
2. Ouvrez le moniteur série à 115200 bauds.
3. Placez le curseur du fader en butée basse.
4. Envoyez la lettre `m` dans le moniteur série.
5. Notez la valeur affichée sous la forme `ADC_RAW_MIN = ...`.
6. Placez le curseur en butée haute.
7. Envoyez la lettre `M`.
8. Notez la valeur affichée sous la forme `ADC_RAW_MAX = ...`.

Conservez ces deux valeurs : elles seront utilisées dans le programme final.

**Important :** si les valeurs sont inversées, le sens de variation du curseur sera inversé. Il est possible de corriger cela en inversant les valeurs MIN et MAX dans le programme final, ou en inversant le calcul.

## 5. Programme Zigbee final

Le programme final doit effectuer les opérations suivantes :

- Lire régulièrement la valeur analogique du fader.
- Convertir cette mesure en pourcentage.
- Exposer cette valeur à travers le cluster Zigbee Analog Input (cluster 0x000C).
- Transmettre les nouvelles valeurs à Zigbee2MQTT.
- Éviter les transmissions inutiles lorsque la position du fader ne change pas significativement.

### Valeurs de calibration

Dans le programme final, remplacez les valeurs suivantes par celles obtenues lors de votre calibration :

```cpp
#define ADC_RAW_MIN 125
#define ADC_RAW_MAX 3337
```

Les valeurs ci-dessus sont des exemples : elles ne doivent pas être conservées sans avoir effectué votre propre calibration.

### Conversion de la mesure ADC en pourcentage

La conversion consiste à ramener la mesure analogique dans une plage comprise entre 0 et 100 :

```cpp
float convertirEnPourcentage(float valeurADC) {
  if (ADC_RAW_MAX <= ADC_RAW_MIN) {
    return 0.0;
  }

  float pourcentage =
      ((valeurADC - ADC_RAW_MIN) * 100.0) /
      (ADC_RAW_MAX - ADC_RAW_MIN);

  return constrain(pourcentage, 0.0, 100.0);
}
```

La lecture peut être moyennée pour réduire les petites variations de mesure :

```cpp
#define POTENTIOMETER_PIN 4
#define ADC_SAMPLES 8

float lireFader() {
  uint32_t total = 0;

  for (uint8_t i = 0; i < ADC_SAMPLES; i++) {
    total += analogRead(POTENTIOMETER_PIN);
  }

  float moyenne = (float)total / ADC_SAMPLES;

  return convertirEnPourcentage(moyenne);
}
```

### Intégration Zigbee

Le programme Zigbee utilise un endpoint dédié et un cluster Analog Input.

Exemple de déclaration :

```cpp
#define SENSOR_ENDPOINT 10

ZigbeeAnalog potentiometerSensor(SENSOR_ENDPOINT);
```

Le cluster Analog Input est ajouté avec :

```cpp
potentiometerSensor.addAnalogInput();
```

La valeur du fader est ensuite publiée à l'aide des fonctions de l'objet `ZigbeeAnalog` utilisées par le programme final.

**Remarque :** conservez les déclarations, l'initialisation Zigbee, les paramètres de reporting et la gestion de la réinitialisation d'usine de votre version fonctionnelle du firmware. Les signatures exactes des méthodes peuvent dépendre de la version du support ESP32 installée dans Arduino IDE.

## 6. Téléversement et appairage Zigbee

Une fois le programme final compilé et téléversé :

1. Redémarrez l'ESP32-H2.
2. Vérifiez que le périphérique démarre correctement.
3. Activez l'autorisation d'appairage dans Zigbee2MQTT.
4. Attendez que le périphérique soit détecté.
5. Donnez-lui un nom explicite, par exemple `fader_esp32_h2`.

Le périphérique doit exposer une valeur numérique correspondant à la position du fader.

### Vérification dans Zigbee2MQTT

Vérifiez que la valeur évolue lorsque vous déplacez le curseur.

Si Zigbee2MQTT reconnaît le périphérique mais ne présente pas la valeur analogique comme une entité exploitable, il peut être nécessaire d'ajouter un convertisseur externe Zigbee2MQTT adapté au cluster Analog Input.

Le cluster utilisé est :

- Cluster : `0x000C`
- Nom : `Analog Input`
- Attribut : `presentValue`

La configuration exacte du convertisseur dépend de la manière dont le périphérique expose ses attributs et de la version de Zigbee2MQTT.

## 7. Intégration dans Home Assistant

Une fois le périphérique correctement reconnu dans Zigbee2MQTT, la valeur numérique doit être disponible dans Home Assistant.

Il faut identifier l'entité correspondant à la valeur du fader.

Par exemple :

```text
sensor.esp32_h2_fader
```

Le nom réel dépendra de votre configuration Zigbee2MQTT et de la découverte MQTT.

Vérifiez dans **Outils de développement > États** que la valeur évolue bien entre 0 et 100 lorsque vous déplacez le curseur.

## 8. Automatisation Home Assistant

Nous allons maintenant créer une automatisation qui adapte la luminosité d'une ampoule Zigbee en fonction de la valeur reçue du fader.

Le comportement souhaité est le suivant :

- À 0 %, l'ampoule s'éteint.
- Entre 1 et 100 %, la luminosité de l'ampoule suit la position du fader.
- Chaque nouvelle valeur reçue déclenche une mise à jour de la luminosité.

### YAML de l'automatisation

Dans Home Assistant, créez une nouvelle automatisation et passez en mode YAML.

```yaml
alias: Variateur Zigbee - Fader ESP32-H2
description: La position du fader règle la luminosité de l'ampoule Zigbee
mode: restart

triggers:
  - trigger: state
    entity_id: sensor.esp32_h2_fader

conditions: []

actions:
  - variables:
      niveau: >
        {{ states('sensor.esp32_h2_fader') | float(0)
           | round(0) | int }}

  - choose:
      - conditions:
          - condition: template
            value_template: "{{ niveau <= 0 }}"
        sequence:
          - action: light.turn_off
            target:
              entity_id: light.ma_lampe_zigbee

    default:
      - action: light.turn_on
        target:
          entity_id: light.ma_lampe_zigbee
        data:
          brightness_pct: "{{ [1, [niveau, 100] | min] | max }}"
          transition: 0.1
```

### Adapter les entités

Deux éléments doivent être remplacés par les identifiants réels de votre installation :

- `sensor.esp32_h2_fader` : l'entité contenant la valeur numérique du fader.
- `light.ma_lampe_zigbee` : l'entité correspondant à votre ampoule Zigbee.

La transition de `0.1` seconde permet de rendre les changements de luminosité plus fluides.

### Fonctionnement de l'automatisation

Lorsque la valeur du capteur change, Home Assistant récupère le pourcentage.

Si la valeur est égale à zéro, l'ampoule est éteinte.

Dans les autres cas, Home Assistant utilise le service `light.turn_on` avec le paramètre `brightness_pct` pour appliquer la luminosité correspondante.

Le mode `restart` permet de relancer l'automatisation à chaque nouvelle valeur reçue.

La réactivité réelle dépend notamment de la fréquence de lecture et de transmission Zigbee configurée dans le firmware.

## 9. Dépannage

### Le fader ne varie pas de 0 à 100 %

Vérifiez les valeurs de calibration :

- `ADC_RAW_MIN`
- `ADC_RAW_MAX`

Elles doivent correspondre aux valeurs mesurées aux deux extrémités de la course mécanique.

### La valeur varie dans le mauvais sens

Inversez les valeurs MIN et MAX ou inversez le calcul du pourcentage.

### Le périphérique apparaît dans Zigbee2MQTT, mais aucune valeur n'est disponible

Vérifiez que le cluster Analog Input est correctement exposé et que Zigbee2MQTT dispose d'un convertisseur adapté si nécessaire.

### Home Assistant ne reçoit pas la valeur

Vérifiez :

- La présence de la valeur dans Zigbee2MQTT.
- La découverte MQTT dans Home Assistant.
- Le nom réel de l'entité.
- L'état du capteur dans les outils de développement de Home Assistant.

### L'ampoule ne réagit pas

Vérifiez que l'identifiant `light.ma_lampe_zigbee` correspond bien à votre ampoule et que celle-ci accepte le contrôle de luminosité.

## 10. Résultat final

Nous avons maintenant un variateur physique Zigbee basé sur un ESP32-H2 et un fader linéaire.

Le fader permet de piloter la luminosité d'une ampoule Zigbee directement depuis Home Assistant, sans avoir besoin d'un variateur Zigbee commercial.

Ce montage peut également servir de base à d'autres commandes analogiques :

- Réglage du volume.
- Contrôle de la température de consigne.
- Réglage de la vitesse d'un ventilateur.
- Contrôle de l'intensité d'un éclairage.
- Création d'interfaces physiques personnalisées pour Home Assistant.

