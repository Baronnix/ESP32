#include "USB.h"
#include "USBMIDI.h"

USBMIDI MIDI;

void setup() {
  USB.begin();              // Active TinyUSB

}

void loop() {
  // Envoie un Do (C4) toutes les 500 ms
  MIDI.noteOn(60, 100, 1);
  delay(500);
  MIDI.noteOff(60, 0, 1);
  delay(500);
}
