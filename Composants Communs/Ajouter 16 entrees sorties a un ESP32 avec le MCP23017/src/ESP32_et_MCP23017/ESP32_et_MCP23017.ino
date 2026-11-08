#include <Wire.h>
#include <Adafruit_MCP23X17.h>

// -----------------------------
// Configuration I2C
// -----------------------------

#define SDA_PIN 4
#define SCL_PIN 5

// Adresse I2C du MCP23017
#define MCP_ADDRESS 0x20

// -----------------------------
// Broches du MCP23017
// -----------------------------

#define LED_PIN 15  // GPB7 : sortie LED

const int INPUT_COUNT = 15;  // GPA0 à GPA7 et GPB0 à GPB6
const unsigned long LED_INTERVAL = 2000; // 2 secondes

Adafruit_MCP23X17 mcp;

// Mémorisation de l'état des 15 entrées
bool previousState[INPUT_COUNT];

// Gestion du clignotement sans delay()
bool ledState = false;
unsigned long previousLedMillis = 0;

// -----------------------------
// Initialisation
// -----------------------------

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("Demarrage MCP23017");

  // Initialisation I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialisation du MCP23017
  if (!mcp.begin_I2C(MCP_ADDRESS, &Wire)) {
    Serial.println("Erreur : MCP23017 introuvable !");
    while (1) {
      delay(100);
    }
  }

  Serial.println("MCP23017 detecte !");

  // Configuration des 15 entrees : GPA0-GPA7 et GPB0-GPB6
  for (int i = 0; i < INPUT_COUNT; i++) {
    mcp.pinMode(i, INPUT_PULLUP);
    previousState[i] = mcp.digitalRead(i);
  }

  // GPB7 (index 15) est reserve a la LED
  mcp.pinMode(LED_PIN, OUTPUT);
  mcp.digitalWrite(LED_PIN, LOW);

  Serial.println("15 entrees configurees.");
  Serial.println("GPB7 configuree en sortie pour la LED.");
  Serial.println("La LED changera d'etat toutes les 2 secondes.");
}

// -----------------------------
// Boucle principale
// -----------------------------

void loop() {

  // Lecture des 15 entrees
  for (int i = 0; i < INPUT_COUNT; i++) {
    bool currentState = mcp.digitalRead(i);

    // Detection d'un changement d'etat
    if (currentState != previousState[i]) {
      previousState[i] = currentState;

      // Les entrees sont en INPUT_PULLUP : LOW = bouton appuye
      if (currentState == LOW) {
        Serial.print("Bouton ");
        Serial.print(i + 1);
        Serial.println(" appuye");
      } else {
        Serial.print("Bouton ");
        Serial.print(i + 1);
        Serial.println(" relache");
      }
    }
  }

  // Clignotement de la LED toutes les 2 secondes
  unsigned long currentMillis = millis();

  if (currentMillis - previousLedMillis >= LED_INTERVAL) {
    previousLedMillis = currentMillis;

    ledState = !ledState;
    mcp.digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }
}