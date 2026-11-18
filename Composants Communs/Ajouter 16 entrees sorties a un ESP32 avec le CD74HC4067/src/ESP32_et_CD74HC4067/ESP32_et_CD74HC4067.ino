#include <Arduino.h>

// =====================================================
// CONFIGURATION MATERIELLE
// =====================================================

// Broches de sélection du CD74HC4067
#define MUX_S0 3
#define MUX_S1 2
#define MUX_S2 1
#define MUX_S3 0

// Broche commune du multiplexeur
// Choisir une broche ADC disponible sur la carte ESP32-H2.
#define MUX_SIG 4

// Voies du CD74HC4067
const uint8_t analogInputChannels[4]   = {0, 1, 2, 3};
const uint8_t digitalInputChannels[4]  = {4, 5, 6, 7};
const uint8_t digitalOutputChannels[4] = {8, 9, 10, 11};
const uint8_t pwmOutputChannels[4]     = {12, 13, 14, 15};

// Valeurs lues
int faderValues[4] = {0, 0, 0, 0};
bool buttonStates[4] = {false, false, false, false};

// Etats souhaites des sorties
bool digitalLedStates[4] = {false, true, false, true};
uint8_t pwmLedValues[4] = {0, 80, 160, 255};

// Temporisation d'affichage
unsigned long previousPrintMillis = 0;
const unsigned long PRINT_INTERVAL = 250;

// =====================================================
// SELECTION D'UNE VOIE DU MULTIPLEXEUR
// =====================================================

void selectMuxChannel(uint8_t channel) {
  channel &= 0x0F;

  digitalWrite(MUX_S0, (channel & 0x01) ? HIGH : LOW);
  digitalWrite(MUX_S1, (channel & 0x02) ? HIGH : LOW);
  digitalWrite(MUX_S2, (channel & 0x04) ? HIGH : LOW);
  digitalWrite(MUX_S3, (channel & 0x08) ? HIGH : LOW);

  // Laisser le temps au signal de se stabiliser.
  delayMicroseconds(50);
}

// =====================================================
// LECTURE DES ENTREES ANALOGIQUES
// =====================================================

void readAnalogInputs() {
  pinMode(MUX_SIG, INPUT);

  for (uint8_t i = 0; i < 4; i++) {
    selectMuxChannel(analogInputChannels[i]);

    // Première lecture écartée pour laisser l'ADC se stabiliser.
    analogRead(MUX_SIG);
    delayMicroseconds(50);

    faderValues[i] = analogRead(MUX_SIG);
  }
}

// =====================================================
// LECTURE DES ENTREES NUMERIQUES
// =====================================================

void readDigitalInputs() {
  pinMode(MUX_SIG, INPUT);

  for (uint8_t i = 0; i < 4; i++) {
    selectMuxChannel(digitalInputChannels[i]);

    buttonStates[i] = (digitalRead(MUX_SIG) == LOW);
  }
}

// =====================================================
// COMMANDE DES SORTIES NUMERIQUES
// =====================================================

void updateDigitalOutputs() {
  for (uint8_t i = 0; i < 4; i++) {
    selectMuxChannel(digitalOutputChannels[i]);

    pinMode(MUX_SIG, OUTPUT);
    digitalWrite(MUX_SIG, digitalLedStates[i] ? HIGH : LOW);
  }

  pinMode(MUX_SIG, INPUT);
}

// =====================================================
// COMMANDE DES SORTIES PWM
// =====================================================

void updatePwmOutputs() {
  for (uint8_t i = 0; i < 4; i++) {
    selectMuxChannel(pwmOutputChannels[i]);

    pinMode(MUX_SIG, OUTPUT);

    // analogWrite génère un signal PWM sur SIG.
    // Le CD74HC4067 le transmet à la voie sélectionnée.
    analogWrite(MUX_SIG, pwmLedValues[i]);
  }

  // Le multiplexeur ne mémorise pas un PWM indépendant par voie.
  pinMode(MUX_SIG, INPUT);
}

// =====================================================
// AFFICHAGE MONITEUR SERIE
// =====================================================

void printValues() {
  Serial.println();
  Serial.println("----- ENTREES ANALOGIQUES -----");

  for (uint8_t i = 0; i < 4; i++) {
    Serial.print("Fader ");
    Serial.print(i + 1);
    Serial.print(" : ");
    Serial.println(faderValues[i]);
  }

  Serial.println("----- ENTREES NUMERIQUES -----");

  for (uint8_t i = 0; i < 4; i++) {
    Serial.print("Bouton ");
    Serial.print(i + 1);
    Serial.print(" : ");
    Serial.println(buttonStates[i] ? "APPUYE" : "RELACHE");
  }
}

// =====================================================
// INITIALISATION
// =====================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("Demarrage CD74HC4067 avec ESP32-H2");

  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);

  pinMode(MUX_SIG, INPUT);

  analogReadResolution(12);

  selectMuxChannel(0);

  Serial.println("CD74HC4067 initialise.");
}

// =====================================================
// BOUCLE PRINCIPALE
// =====================================================

void loop() {
  readAnalogInputs();
  readDigitalInputs();

  updateDigitalOutputs();
  updatePwmOutputs();

  unsigned long currentMillis = millis();

  if (currentMillis - previousPrintMillis >= PRINT_INTERVAL) {
    previousPrintMillis = currentMillis;
    printValues();
  }
}