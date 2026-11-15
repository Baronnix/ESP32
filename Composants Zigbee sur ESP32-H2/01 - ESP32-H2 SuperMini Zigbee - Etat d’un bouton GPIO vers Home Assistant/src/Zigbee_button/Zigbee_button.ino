#include <Arduino.h>
#include "Zigbee.h"

// --------------------------------------------------
// CONFIGURATION
// --------------------------------------------------

#define SENSOR_ENDPOINT 10

#define EXTERNAL_BUTTON_PIN 14
#define FACTORY_RESET_PIN 9

const uint32_t DEBOUNCE_MS = 50;
const uint32_t FACTORY_RESET_HOLD_MS = 5000;

// --------------------------------------------------
// ZIGBEE BINARY INPUT
// --------------------------------------------------

ZigbeeBinary buttonSensor(SENSOR_ENDPOINT);

// --------------------------------------------------
// VARIABLES BOUTON GPIO14
// --------------------------------------------------

bool lastRawButtonState = false;
bool stableButtonState = false;
uint32_t lastDebounceTime = 0;

// --------------------------------------------------
// VARIABLES FACTORY RESET GPIO9
// --------------------------------------------------

uint32_t bootButtonPressTime = 0;
bool factoryResetTriggered = false;

// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("Demarrage ESP32-H2 Zigbee");

  // Initialisation des boutons
  pinMode(EXTERNAL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);

  // Initialisation de l'etat du bouton GPIO14
  lastRawButtonState = digitalRead(EXTERNAL_BUTTON_PIN) == LOW;
  stableButtonState = lastRawButtonState;

  // Configuration du capteur Zigbee
  buttonSensor.setManufacturerAndModel(
    "DIY",
    "ESP32-H2-GPIO14-Button"
  );

  buttonSensor.addBinaryInput();

  buttonSensor.setBinaryInputApplication(
    BINARY_INPUT_APPLICATION_TYPE_HVAC_OTHER
  );

  buttonSensor.setBinaryInputDescription("Bouton GPIO14");

  buttonSensor.setBinaryInput(stableButtonState);

  // Ajout de l'endpoint
  Zigbee.addEndpoint(&buttonSensor);

  // Configuration Zigbee
  Zigbee.setDebugMode(true);
  Zigbee.setPrimaryChannelMask(1UL << 11);

  // Demarrage Zigbee
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

  bool bootButtonPressed = (digitalRead(FACTORY_RESET_PIN) == LOW);

  if (bootButtonPressed && !factoryResetTriggered) {

    // Premier appui
    if (bootButtonPressTime == 0) {
      bootButtonPressTime = millis();
      Serial.println("Bouton BOOT : appui detecte");
    }

    // Maintien pendant 5 secondes
    if (millis() - bootButtonPressTime >= FACTORY_RESET_HOLD_MS) {

      factoryResetTriggered = true;

      Serial.println();
      Serial.println("FACTORY RESET ZIGBEE !");
      Serial.println("Effacement du reseau et redemarrage...");

      delay(500);

      // Efface les parametres Zigbee et redemarre
      Zigbee.factoryReset(true);

      // Securite si la fonction ne redemarre pas
      while (true) {
        delay(1000);
      }
    }

  } else if (!bootButtonPressed) {

    // Relachement du bouton BOOT
    bootButtonPressTime = 0;
  }

  // ================================================
  // 2. BOUTON EXTERNE GPIO14 : BINARY INPUT
  // ================================================

  bool rawButtonState = digitalRead(EXTERNAL_BUTTON_PIN) == LOW;

  // Detection changement brut
  if (rawButtonState != lastRawButtonState) {

    lastRawButtonState = rawButtonState;
    lastDebounceTime = millis();
  }

  // Anti-rebond
  if (millis() - lastDebounceTime >= DEBOUNCE_MS &&
      rawButtonState != stableButtonState) {

    stableButtonState = rawButtonState;

    // Mise a jour du cluster
    buttonSensor.setBinaryInput(stableButtonState);

    Serial.printf(
      "Bouton GPIO14 : %s\n",
      stableButtonState ? "APPUYE" : "RELACHE"
    );

    // Publication Zigbee
    bool result = buttonSensor.reportBinaryInput();

    Serial.printf(
      "Publication Zigbee : %s\n",
      result ? "OK" : "ECHEC"
    );
  }

  // ================================================
  // 3. SURVEILLANCE CONNEXION ZIGBEE
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