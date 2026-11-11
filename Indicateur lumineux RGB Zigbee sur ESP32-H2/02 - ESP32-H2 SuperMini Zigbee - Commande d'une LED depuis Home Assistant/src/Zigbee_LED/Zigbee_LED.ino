#include <Arduino.h>
#include "Zigbee.h"

// Vérifie que le mode Zigbee End Device est sélectionné.
#ifndef ZIGBEE_MODE_ED
#error "Sélectionner Tools > Zigbee mode > Zigbee ED (end device)"
#endif

// --------------------------------------------------
// CONFIGURATION
// --------------------------------------------------

#define LIGHT_ENDPOINT       10
#define LED_PIN              12
#define FACTORY_RESET_PIN    9

const uint32_t FACTORY_RESET_HOLD_MS = 5000;

// Avec le câblage proposé : HIGH = LED allumée.
const bool LED_ACTIVE_HIGH = true;

// --------------------------------------------------
// ZIGBEE ON/OFF LIGHT
// --------------------------------------------------

ZigbeeLight zbLight(LIGHT_ENDPOINT);

// --------------------------------------------------
// VARIABLES FACTORY RESET
// --------------------------------------------------

uint32_t bootButtonPressTime = 0;
bool factoryResetTriggered = false;

// --------------------------------------------------
// CALLBACK : COMMANDE ON/OFF REÇUE
// --------------------------------------------------

void applyLedState(bool state) {
  const uint8_t outputLevel =
    (state == LED_ACTIVE_HIGH) ? HIGH : LOW;

  digitalWrite(LED_PIN, outputLevel);

  Serial.printf(
    "Commande Zigbee reçue : lumière %s -> GPIO%d = %s\n",
    state ? "ALLUMÉE" : "ÉTEINTE",
    LED_PIN,
    outputLevel == HIGH ? "HIGH" : "LOW"
  );
}

// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("Démarrage ESP32-H2 - Zigbee On/Off Light");

  // Initialisation de la sortie, LED éteinte.
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_ACTIVE_HIGH ? LOW : HIGH);

  // Bouton BOOT pour le Factory Reset.
  pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);

  // Identification dans Zigbee2MQTT.
  zbLight.setManufacturerAndModel(
    "DIY",
    "ESP32-H2-GPIO12-Light"
  );

  // Le callback est appelé lors d'un changement d'état.
  zbLight.onLightChange(applyLedState);

  // Ajout de l'endpoint lumière à la pile Zigbee.
  Zigbee.addEndpoint(&zbLight);

  // Configuration Zigbee.
  Zigbee.setDebugMode(true);
  Zigbee.setPrimaryChannelMask(1UL << 11);

  // Démarrage de Zigbee.
  if (!Zigbee.begin()) {
    Serial.println("Échec du démarrage Zigbee. Redémarrage...");
    delay(1000);
    ESP.restart();
  }

  Serial.println("Pile Zigbee démarrée.");
  Serial.println("En attente de connexion au réseau Zigbee...");
}

// --------------------------------------------------
// LOOP
// --------------------------------------------------

void loop() {
  // ================================================
  // 1. BOUTON BOOT GPIO9 : FACTORY RESET
  // ================================================

  const bool bootButtonPressed =
    (digitalRead(FACTORY_RESET_PIN) == LOW);

  if (bootButtonPressed && !factoryResetTriggered) {
    if (bootButtonPressTime == 0) {
      bootButtonPressTime = millis();
      Serial.println("Bouton BOOT : appui détecté");
    }

    if (millis() - bootButtonPressTime >= FACTORY_RESET_HOLD_MS) {
      factoryResetTriggered = true;

      Serial.println();
      Serial.println("FACTORY RESET ZIGBEE !");
      Serial.println("Effacement du réseau et redémarrage...");

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
  // 2. SURVEILLANCE DE LA CONNEXION ET DE L'ÉTAT
  // ================================================

  static uint32_t lastLog = 0;

  if (millis() - lastLog >= 5000) {
    lastLog = millis();

    Serial.printf(
      "Zigbee connecté : %s | État lumière : %s\n",
      Zigbee.connected() ? "OUI" : "NON",
      zbLight.getLightState() ? "ON" : "OFF"
    );
  }

  delay(10);
}