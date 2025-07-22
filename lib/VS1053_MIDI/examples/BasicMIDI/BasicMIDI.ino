/**
 * BasicMIDI - Simple example for VS1053_MIDI library
 * 
 * This example demonstrates basic MIDI functionality including:
 * - Library initialization with MIDI plugin loading
 * - Instrument changes
 * - Note playing with different velocities
 * - Volume control
 * 
 * Hardware connections (Arduino Nano):
 * - MISO: Pin 12
 * - MOSI: Pin 11  
 * - SCK:  Pin 13
 * - DREQ: Pin 2
 * - XRST: Pin 8
 * - XCS:  Pin 6
 * - XDCS: Pin 7
 */

#include <SPI.h>
#include <VS1053_MIDI.h>

// Pin definitions for VS1053 module
#define VS1053_XCS   6    // Chip Select for commands (active low)
#define VS1053_XDCS  7    // Chip Select for data (active low)
#define VS1053_DREQ  2    // Data Request (indicates ready for more data)
#define VS1053_RESET 8    // Reset pin (active low)

// Create VS1053_MIDI instance
VS1053_MIDI midi(VS1053_XCS, VS1053_XDCS, VS1053_DREQ, VS1053_RESET);

void setup() {
  Serial.begin(115200);
  Serial.println("VS1053 MIDI Library - Basic Example");
  
  // Initialize SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16); // Slower clock for better stability
  
  // Initialize VS1053 with MIDI plugin
  if (midi.begin(true)) { // true = load MIDI plugin for full GM support
    Serial.println("VS1053 MIDI initialized successfully!");
  } else {
    Serial.println("VS1053 MIDI initialization failed!");
    while(1); // Stop here if initialization fails
  }
  
  // Set master volume (0x00 = loudest, 0xFE = quietest)
  midi.setMasterVolume(0x20, 0x20);
  
  Serial.println("Playing demonstration...");
}

void loop() {
  // Demonstrate different instruments and notes
  
  // Piano (GM instrument 0)
  Serial.println("Playing Piano...");
  midi.setInstrument(0, GM_Instruments::ACOUSTIC_GRAND_PIANO);
  playScale(0);
  delay(1000);
  
  // Organ (GM instrument 16)
  Serial.println("Playing Organ...");
  midi.setInstrument(0, GM_Instruments::DRAWBAR_ORGAN);
  playChord(0);
  delay(1000);
  
  // Flute (GM instrument 73)
  Serial.println("Playing Flute...");
  midi.setInstrument(0, GM_Instruments::FLUTE);
  playMelody(0);
  delay(1000);
  
  // Tubular Bells (GM instrument 14)
  Serial.println("Playing Tubular Bells...");
  midi.setInstrument(0, GM_Instruments::TUBULAR_BELLS);
  playBells(0);
  delay(2000);
  
  Serial.println("Demonstration complete, repeating...\n");
}

void playScale(uint8_t channel) {
  // Play a C major scale
  uint8_t notes[] = {60, 62, 64, 65, 67, 69, 71, 72}; // C4 to C5
  
  for (int i = 0; i < 8; i++) {
    midi.noteOn(channel, notes[i], 80);
    delay(300);
    midi.noteOff(channel, notes[i], 64);
    delay(50);
  }
}

void playChord(uint8_t channel) {
  // Play a C major chord
  midi.noteOn(channel, 60, 80);  // C4
  midi.noteOn(channel, 64, 80);  // E4
  midi.noteOn(channel, 67, 80);  // G4
  delay(1000);
  midi.noteOff(channel, 60, 64);
  midi.noteOff(channel, 64, 64);
  midi.noteOff(channel, 67, 64);
}

void playMelody(uint8_t channel) {
  // Simple melody: Twinkle Twinkle Little Star (first phrase)
  uint8_t notes[] = {60, 60, 67, 67, 69, 69, 67}; // C C G G A A G
  uint16_t durations[] = {400, 400, 400, 400, 400, 400, 800};
  
  for (int i = 0; i < 7; i++) {
    midi.noteOn(channel, notes[i], 70);
    delay(durations[i] * 0.8); // Play for 80% of duration
    midi.noteOff(channel, notes[i], 64);
    delay(durations[i] * 0.2); // Rest for 20% of duration
  }
}

void playBells(uint8_t channel) {
  // Play some bell notes with varying velocities
  uint8_t notes[] = {72, 76, 79, 84}; // C5, E5, G5, C6
  
  for (int i = 0; i < 4; i++) {
    uint8_t velocity = 60 + (i * 15); // Increase velocity each time
    midi.noteOn(channel, notes[i], velocity);
    delay(500);
    midi.noteOff(channel, notes[i], 64);
    delay(300);
  }
}
