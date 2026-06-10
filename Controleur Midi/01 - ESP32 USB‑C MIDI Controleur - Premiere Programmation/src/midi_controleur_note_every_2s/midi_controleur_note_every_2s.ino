#include "Adafruit_TinyUSB.h"

Adafruit_USBD_MIDI usb_midi;

void setup() {
  usb_midi.begin();
  delay(1000);
}

void loop() {
  uint8_t note = 60;      // C4
  uint8_t velocity = 100; // Vélocité

  // NOTE ON (canal 1 → 0x90)
  uint8_t msgOn[3] = {0x90, note, velocity};
  usb_midi.write(msgOn, 3);

  delay(500);

  // NOTE OFF (canal 1 → 0x80)
  uint8_t msgOff[3] = {0x80, note, velocity};
  usb_midi.write(msgOff, 3);

  delay(1000);
}
