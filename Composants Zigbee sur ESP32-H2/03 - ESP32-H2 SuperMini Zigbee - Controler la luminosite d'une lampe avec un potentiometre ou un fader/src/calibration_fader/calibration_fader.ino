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