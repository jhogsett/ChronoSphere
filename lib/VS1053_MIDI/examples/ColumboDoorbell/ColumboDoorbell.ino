/**
 * Columbo Doorbell - VS1053_MIDI Library Example
 * 
 * Recreates the iconic three-note doorbell chime from the TV show "Columbo".
 * This sophisticated chime uses a descending G Major triad (G-E-C) with 
 * carefully timed pauses to create that distinctive "high-priced house" sound.
 * 
 * AUTHENTIC DOORBELL RESEARCH:
 * Traditional doorbells use two resonators tuned to "pleasing notes" - typically
 * a major third or perfect fifth interval. The Columbo chime extends this to 
 * three notes, creating a complete triad resolution.
 * 
 * Musical Analysis:
 * - Note 1: G4 (MIDI 67) - The "Ding" (5th of C Major)
 * - Note 2: E4 (MIDI 64) - The "Dong" (3rd of C Major)
 * - Note 3: C4 (MIDI 60) - The final resonant tone (Root - resolution)
 * 
 * This creates a descending G Major triad in first inversion, which resolves
 * to the tonic (C), giving it that sophisticated, "finished" sound that 
 * distinguished expensive homes in the 1970s.
 * 
 * Hardware:
 * - Arduino Nano (or compatible)
 * - VS1053 Module
 * 
 * Pin Connections (Arduino Nano):
 * - MISO: Pin 12
 * - MOSI: Pin 11  
 * - SCK: Pin 13
 * - DREQ: Pin 2
 * - XRST: Pin 8
 * - XCS: Pin 6
 * - XDCS: Pin 7
 * 
 * Author: VS1053_MIDI Library
 * License: MIT
 */

#include <SPI.h>
#include <VS1053_MIDI.h>

// Pin definitions
#define VS1053_XCS   6
#define VS1053_XDCS  7
#define VS1053_DREQ  2
#define VS1053_RESET 8

// Create VS1053_MIDI instance
VS1053_MIDI midi(VS1053_XCS, VS1053_XDCS, VS1053_DREQ, VS1053_RESET);

// Columbo doorbell notes (G-E-C descending triad)
const uint8_t COLUMBO_NOTE_1 = 67; // G4 - The "Ding"
const uint8_t COLUMBO_NOTE_2 = 64; // E4 - The "Dong" 
const uint8_t COLUMBO_NOTE_3 = 60; // C4 - The final resonant tone

// Timing constants (in milliseconds) - Equal timing for authentic mechanical precision
const uint16_t NOTE_DURATION = 165;  // How long each note plays (18% faster total)
const uint16_t PAUSE_BETWEEN = 621;  // Equal pause between all notes (18% faster total)
const uint16_t FINAL_SUSTAIN = 2474; // Let final C note ring out naturally (18% faster total)
const uint16_t CHIME_INTERVAL = 8000; // Time between doorbell rings

// MIDI settings
const uint8_t MIDI_CHANNEL = 0;
const uint8_t NOTE_VELOCITY = 100;   // Moderate velocity for elegant sound

void setup() {
  Serial.begin(115200);
  Serial.println("=== Columbo Doorbell Example ===");
  Serial.println("VS1053_MIDI Library - Iconic TV Doorbell");
  Serial.println();
  Serial.println("Recreating the sophisticated three-note chime");
  Serial.println("from the classic TV show 'Columbo'");
  Serial.println();
  
  // Initialize SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  
  // Initialize VS1053 with MIDI plugin
  if (midi.begin(true)) {
    Serial.println("✓ VS1053_MIDI initialized successfully!");
    Serial.print("Plugin size: ");
    Serial.print(midi.getPluginSize());
    Serial.println(" words");
    Serial.println();
  } else {
    Serial.println("✗ VS1053_MIDI initialization failed!");
    while(1);
  }
  
  // Set volume for elegant chime sound
  midi.setMasterVolume(0x20, 0x20);
  
  // Use Tubular Bells instrument for authentic chime sound
  midi.setInstrument(MIDI_CHANNEL, GM_Instruments::TUBULAR_BELLS);
  delay(100);
  
  Serial.println("Starting Columbo doorbell demonstration...");
  Serial.println("Notes: G4 (67) - E4 (64) - C4 (60)");
  Serial.println("Chime will repeat every 5 seconds");
  Serial.println();
  
  delay(1000);
}

void loop() {
  playColumboDoorbell();
  delay(CHIME_INTERVAL);
}

/**
 * Play the iconic Columbo doorbell sequence
 * 
 * This function recreates the three-note descending chime with
 * authentic timing that captures the sophisticated, unhurried 
 * feel of the original TV show doorbell.
 */
void playColumboDoorbell() {
  Serial.println("🔔 *DING*... *DONG*... *DONG* 🏠");
  Serial.print("  Playing: G4");
  
  // Note 1: G4 - The "Ding"
  midi.noteOn(MIDI_CHANNEL, COLUMBO_NOTE_1, NOTE_VELOCITY);
  delay(NOTE_DURATION);
  midi.noteOff(MIDI_CHANNEL, COLUMBO_NOTE_1, 64);
  delay(PAUSE_BETWEEN);
  
  Serial.print(" -> E4");
  
  // Note 2: E4 - The "Dong"  
  midi.noteOn(MIDI_CHANNEL, COLUMBO_NOTE_2, NOTE_VELOCITY);
  delay(NOTE_DURATION);
  midi.noteOff(MIDI_CHANNEL, COLUMBO_NOTE_2, 64);
  delay(PAUSE_BETWEEN);
  
  Serial.print(" -> C4");
  
  // Note 3: C4 - The final resonant tone
  midi.noteOn(MIDI_CHANNEL, COLUMBO_NOTE_3, NOTE_VELOCITY);
  delay(FINAL_SUSTAIN);
  midi.noteOff(MIDI_CHANNEL, COLUMBO_NOTE_3, 64);
  
  Serial.println(" (sustain...)");
  Serial.println("  \"Just one more thing...\" 🕵️");
  Serial.println();
}

/**
 * Alternative version with different octave for deeper chime
 * Uncomment and call this instead of playColumboDoorbell() for variation
 */
/*
void playColumboDoorbellDeep() {
  const uint8_t G3 = 55; // G3 - Lower octave
  const uint8_t E3 = 52; // E3 
  const uint8_t C3 = 48; // C3
  
  Serial.println("🔔 *DING*... *DONG*... *DONG* (Deep) 🏠");
  
  midi.noteOn(MIDI_CHANNEL, G3, NOTE_VELOCITY);
  delay(PAUSE_AFTER_G);
  midi.noteOff(MIDI_CHANNEL, G3, 64);
  
  midi.noteOn(MIDI_CHANNEL, E3, NOTE_VELOCITY);
  delay(PAUSE_AFTER_E);
  midi.noteOff(MIDI_CHANNEL, E3, 64);
  
  midi.noteOn(MIDI_CHANNEL, C3, NOTE_VELOCITY);
  delay(FINAL_SUSTAIN);
  midi.noteOff(MIDI_CHANNEL, C3, 64);
}
*/
