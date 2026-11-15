#include <Arduino.h>
#include "Zigbee.h"

// --------------------------------------------------
// CONFIGURATION
// --------------------------------------------------

#define SENSOR_ENDPOINT 10

#define POTENTIOMETER_PIN 4
#define FACTORY_RESET_PIN 9

// Résolution ADC
#define ADC_RESOLUTION 12
#define ADC_MAX_VALUE 4095

// Temporisations
const uint32_t READ_INTERVAL_MS = 50;
const uint32_t FACTORY_RESET_HOLD_MS = 5000;

// Filtrage et publication
const uint8_t ADC_SAMPLES = 8;
const float REPORT_THRESHOLD = 1.0;
const uint32_t MAX_REPORT_INTERVAL_MS = 30000;

// --------------------------------------------------
// ZIGBEE ANALOG INPUT
// --------------------------------------------------

ZigbeeAnalog potentiometerSensor(SENSOR_ENDPOINT);

// --------------------------------------------------
// VARIABLES POTENTIOMETRE
// --------------------------------------------------

float currentPercentage = 0;
float lastReportedPercentage = -1;

uint32_t lastReadTime = 0;
uint32_t lastReportTime = 0;

// --------------------------------------------------
// VARIABLES FACTORY RESET GPIO9
// --------------------------------------------------

uint32_t bootButtonPressTime = 0;
bool factoryResetTriggered = false;

// --------------------------------------------------
// LECTURE ANALOGIQUE
// --------------------------------------------------

float readPotentiometer() {

  uint32_t total = 0;

  // Moyenne de plusieurs mesures
  for (uint8_t i = 0; i < ADC_SAMPLES; i++) {
    total += analogRead(POTENTIOMETER_PIN);
  }

  float average = (float)total / ADC_SAMPLES;

  // Conversion en pourcentage
  float percentage = (average / ADC_MAX_VALUE) * 100.0;

  // Limitation entre 0 et 100
  percentage = constrain(percentage, 0.0, 100.0);

  return percentage;
}

// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("Demarrage ESP32-H2 Zigbee Fader");

  // Initialisation des broches
  pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);

  // Configuration ADC
  analogReadResolution(ADC_RESOLUTION);

  // Identification Zigbee
  potentiometerSensor.setManufacturerAndModel(
    "DIY",
    "ESP32-H2-Zigbee-Fader"
  );

  // Création du cluster Analog Input
  potentiometerSensor.addAnalogInput();

  // Type de donnée : pourcentage
  potentiometerSensor.setAnalogInputApplication(
    ESP_ZB_ZCL_AI_PERCENTAGE_OTHER
  );

  potentiometerSensor.setAnalogInputDescription(
    "Luminosite potentiometre"
  );

  // Résolution : 1 %
  potentiometerSensor.setAnalogInputResolution(1.0);

  // Valeurs minimale et maximale
  potentiometerSensor.setAnalogInputMinMax(0.0, 100.0);

  // Lecture initiale
  currentPercentage = readPotentiometer();

  // Initialisation de la valeur Zigbee
  potentiometerSensor.setAnalogInput(currentPercentage);

  // Ajout de l'endpoint
  Zigbee.addEndpoint(&potentiometerSensor);

  // Configuration Zigbee
  Zigbee.setDebugMode(true);
  Zigbee.setPrimaryChannelMask(1UL << 11);

  // Démarrage Zigbee
  Zigbee.begin();

  Serial.println("Initialisation Zigbee terminee");
}

// --------------------------------------------------
// LOOP
// --------------------------------------------------

void loop() {

  // ================================================
  // 1. BOUTON BOOT GPIO9 : FACTORY RESET
  // ================================================

  bool bootButtonPressed =
    (digitalRead(FACTORY_RESET_PIN) == LOW);

  if (bootButtonPressed && !factoryResetTriggered) {

    if (bootButtonPressTime == 0) {
      bootButtonPressTime = millis();
      Serial.println("Bouton BOOT : appui detecte");
    }

    if (millis() - bootButtonPressTime >=
        FACTORY_RESET_HOLD_MS) {

      factoryResetTriggered = true;

      Serial.println();
      Serial.println("FACTORY RESET ZIGBEE !");
      Serial.println("Effacement du reseau et redemarrage...");

      delay(500);

      Zigbee.factoryReset(true);

      while (true) {
        delay(1000);
      }
    }

  } else if (!bootButtonPressed) {

    bootButtonPressTime = 0;
  }

  // ================================================
  // 2. LECTURE DU POTENTIOMETRE
  // ================================================

  if (millis() - lastReadTime >= READ_INTERVAL_MS) {

    lastReadTime = millis();

    float newPercentage = readPotentiometer();

    // Mise à jour de la valeur courante
    currentPercentage = newPercentage;

    // ==============================================
    // 3. FILTRAGE ET PUBLICATION ZIGBEE
    // ==============================================

    float difference =
      abs(currentPercentage - lastReportedPercentage);

    bool valueChanged =
      difference >= REPORT_THRESHOLD;

    bool periodicReport =
      millis() - lastReportTime >= MAX_REPORT_INTERVAL_MS;

    if (valueChanged || periodicReport) {

      // Mise à jour du cluster
      potentiometerSensor.setAnalogInput(
        currentPercentage
      );

      Serial.printf(
        "Position potentiometre : %.1f %%\n",
        currentPercentage
      );

      // Publication Zigbee
      if (Zigbee.connected()) {

        bool result =
          potentiometerSensor.reportAnalogInput();

        Serial.printf(
          "Publication Zigbee : %s\n",
          result ? "OK" : "ECHEC"
        );

        if (result) {
          lastReportedPercentage = currentPercentage;
          lastReportTime = millis();
        }
      }
    }
  }

  // ================================================
  // 4. SURVEILLANCE CONNEXION ZIGBEE
  // ================================================

  static uint32_t lastLog = 0;

  if (millis() - lastLog >= 2000) {

    lastLog = millis();

    Serial.printf(
      "Zigbee connected : %d\n",
      Zigbee.connected()
    );
  }

  delay(10);
}