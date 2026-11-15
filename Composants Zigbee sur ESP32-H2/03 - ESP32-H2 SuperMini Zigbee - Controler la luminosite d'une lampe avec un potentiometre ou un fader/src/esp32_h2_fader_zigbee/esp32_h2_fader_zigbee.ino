
#include <Arduino.h>
#include "Zigbee.h"

// =====================================================
// CONFIGURATION MATERIELLE
// =====================================================

#define SENSOR_ENDPOINT       10
#define POTENTIOMETER_PIN     4
#define FACTORY_RESET_PIN     9

// =====================================================
// CALIBRATION DU FADER
// =====================================================

// IMPORTANT : remplacer par les valeurs obtenues
// avec le programme de calibration.

#define ADC_RAW_MIN           125
#define ADC_RAW_MAX           3337

// =====================================================
// PARAMETRES DE LECTURE ET DE TRANSMISSION
// =====================================================

#define ADC_SAMPLES           8

// Lecture du fader toutes les 50 ms
#define READ_INTERVAL_MS      50

// Variation minimale pour transmettre une nouvelle valeur
#define REPORT_THRESHOLD      1.0f

// Transmission périodique de sécurité
#define MAX_REPORT_INTERVAL_MS 30000

// Appui long pour réinitialiser Zigbee
#define FACTORY_RESET_HOLD_MS 5000

// =====================================================
// OBJET ZIGBEE
// =====================================================

ZigbeeAnalog potentiometerSensor(SENSOR_ENDPOINT);

// =====================================================
// VARIABLES
// =====================================================

float currentPercentage = 0.0f;
float lastReportedPercentage = -1000.0f;

uint32_t lastReadTime = 0;
uint32_t lastReportTime = 0;
uint32_t buttonPressTime = 0;

bool buttonWasPressed = false;
bool zigbeeConnected = false;

// =====================================================
// LECTURE DU FADER
// =====================================================

float readPotentiometer() {

  uint32_t total = 0;

  // Moyenne de plusieurs mesures pour stabiliser l'ADC
  for (uint8_t i = 0; i < ADC_SAMPLES; i++) {
    total += analogRead(POTENTIOMETER_PIN);
  }

  float average = (float)total / ADC_SAMPLES;

  return average;
}

// =====================================================
// CONVERSION ADC VERS POURCENTAGE
// =====================================================

float convertToPercentage(float rawValue) {

  if (ADC_RAW_MAX <= ADC_RAW_MIN) {
    return 0.0f;
  }

  float percentage =
    ((rawValue - ADC_RAW_MIN) * 100.0f) /
    (ADC_RAW_MAX - ADC_RAW_MIN);

  // Limitation entre 0 et 100 %
  percentage = constrain(percentage, 0.0f, 100.0f);

  return percentage;
}

// =====================================================
// GESTION DE LA REINITIALISATION ZIGBEE
// =====================================================

void checkFactoryResetButton() {

  bool pressed = (digitalRead(FACTORY_RESET_PIN) == LOW);

  if (pressed && !buttonWasPressed) {

    buttonPressTime = millis();
    buttonWasPressed = true;

  }

  if (!pressed && buttonWasPressed) {

    buttonWasPressed = false;

  }

  if (pressed && buttonWasPressed) {

    if (millis() - buttonPressTime >= FACTORY_RESET_HOLD_MS) {

      Serial.println();
      Serial.println("Reinitialisation Zigbee...");
      Serial.println("Effacement de la configuration reseau.");

      delay(500);

      Zigbee.factoryReset();

      delay(1000);

      ESP.restart();
    }
  }
}

// =====================================================
// TRANSMISSION DE LA VALEUR ZIGBEE
// =====================================================

void reportPercentage(float percentage) {

  // Mise à jour de la valeur du cluster Analog Input
  potentiometerSensor.setAnalogInput(percentage);

  bool success = potentiometerSensor.reportAnalogInput();

  if (success) {

    lastReportedPercentage = percentage;
    lastReportTime = millis();

    Serial.print("Valeur Zigbee envoyee : ");
    Serial.print(percentage, 1);
    Serial.println(" %");

  } else {

    Serial.println("Erreur lors de la transmission Zigbee");

  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=================================");
  Serial.println("   FADER ZIGBEE ESP32-H2");
  Serial.println("=================================");

  // Configuration de l'entrée analogique
  analogReadResolution(12);

  pinMode(POTENTIOMETER_PIN, INPUT);

  // Configuration du bouton de réinitialisation
  pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);

  // ---------------------------------------------------
  // CONFIGURATION DU CLUSTER ANALOG INPUT
  // ---------------------------------------------------

  potentiometerSensor.setManufacturerAndModel(
    "Baronnix",
    "Fader Zigbee ESP32-H2"
  );

  // Ajout du cluster Analog Input
  potentiometerSensor.addAnalogInput();

  // Type de donnée : pourcentage
  potentiometerSensor.setAnalogInputApplication(
    ESP_ZB_ZCL_AI_PERCENTAGE_OTHER
  );

  potentiometerSensor.setAnalogInputDescription(
    "Position du fader"
  );

  // Résolution : 1 %
  potentiometerSensor.setAnalogInputResolution(1.0f);

  // Plage de valeurs : 0 à 100
  potentiometerSensor.setAnalogInputMinMax(0.0f, 100.0f);

  // Configuration du reporting Zigbee
  potentiometerSensor.setAnalogInputReporting(
    1,
    30,
    REPORT_THRESHOLD
  );

  // Enregistrement de l'endpoint
  Zigbee.addEndpoint(&potentiometerSensor);

  // ---------------------------------------------------
  // DEMARRAGE ZIGBEE
  // ---------------------------------------------------

  Serial.println("Demarrage du reseau Zigbee...");

  
  if (!Zigbee.begin(ZIGBEE_END_DEVICE)) {  //ZIGBEE_END_DEVICE or ZIGBEE_ROUTER

    Serial.println("Erreur de demarrage Zigbee !");
    delay(1000);
    ESP.restart();

  }

  Serial.println("Zigbee demarre.");
  Serial.println("Attente de la connexion au coordinateur...");

  while (!Zigbee.connected()) {

    Serial.print(".");
    delay(200);

    checkFactoryResetButton();

  }

  zigbeeConnected = true;

  Serial.println();
  Serial.println("Connexion Zigbee etablie !");

  // Lecture initiale du fader
  currentPercentage = convertToPercentage(
    readPotentiometer()
  );

  // Transmission de la première valeur
  reportPercentage(currentPercentage);

}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // Gestion du bouton de réinitialisation
  checkFactoryResetButton();

  // Vérification de la connexion Zigbee
  if (!zigbeeConnected || !Zigbee.connected()) {
    delay(10);
    return;
  }

  // Lecture périodique du fader
  if (millis() - lastReadTime >= READ_INTERVAL_MS) {

    lastReadTime = millis();

    float rawValue = readPotentiometer();

    currentPercentage = convertToPercentage(rawValue);

    Serial.print("ADC : ");
    Serial.print(rawValue, 0);

    Serial.print(" | Position : ");
    Serial.print(currentPercentage, 1);

    Serial.println(" %");

    // Mise à jour de la valeur locale du cluster
    potentiometerSensor.setAnalogInput(currentPercentage);

    // Vérification du seuil de variation
    bool valueChanged =
      abs(currentPercentage - lastReportedPercentage)
      >= REPORT_THRESHOLD;

    // Transmission périodique même si la valeur ne change pas
    bool periodicReport =
      millis() - lastReportTime >= MAX_REPORT_INTERVAL_MS;

    if (valueChanged || periodicReport) {

      reportPercentage(currentPercentage);

    }
  }

  delay(5);
}
