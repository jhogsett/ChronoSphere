/**
 * Iconic Movie Sounds - VS1053_MIDI Library Example
 * 
 * Recreates famous musical motifs from classic films using MIDI synthesis.
 * These simple yet powerful note combinations have become cultural touchstones
 * that instantly evoke their respective movies.
 * 
 * Featured Sounds:
 * 1. JAWS - The menacing two-note shark theme (E-F alternating)
 * 2. PSYCHO - The piercing violin stabs from the shower scene 
 * 3. CLOSE ENCOUNTERS - The alien communication 5-note sequence
 * 4. THE TWILIGHT ZONE - The eerie descending theme
 * 5. SUPERMAN - The heroic ascending fanfare opening
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

// MIDI settings
const uint8_t MIDI_CHANNEL = 0;

// === JAWS Theme (1975) ===
// The iconic two-note shark approach theme
const uint8_t JAWS_NOTE_1 = 40;  // E2 - Low, menacing
const uint8_t JAWS_NOTE_2 = 41;  // F2 - Half-step up creates tension
const uint16_t JAWS_SLOW_TEMPO = 800;    // Starts slow and ominous
const uint16_t JAWS_FAST_TEMPO = 100;    // Builds to frantic speed

// === PSYCHO Shower Scene (1960) ===
// Bernard Herrmann's piercing violin stabs
const uint8_t PSYCHO_NOTES[] = {
  73, 69, 76, 72, 78, 74, 81, 77  // High register dissonant intervals
};
const uint8_t PSYCHO_NUM_NOTES = 8;
const uint16_t PSYCHO_STAB_DURATION = 80;   // Sharp, quick stabs
const uint16_t PSYCHO_PAUSE = 120;          // Brief pause between stabs

// === CLOSE ENCOUNTERS Communication (1977) ===
// The 5-note alien communication sequence: D-E-C-C(low)-G
const uint8_t CE_NOTES[] = {62, 64, 60, 48, 55}; // D4-E4-C4-C3-G3
const uint8_t CE_NUM_NOTES = 5;
const uint16_t CE_NOTE_DURATION = 400;     // Measured, deliberate timing
const uint16_t CE_NOTE_PAUSE = 100;        // Short pause between notes

// === TWILIGHT ZONE Theme (1959) ===
// The eerie, otherworldly descending pattern
const uint8_t TZ_NOTES[] = {72, 67, 64, 60, 55}; // C5 down to G3
const uint8_t TZ_NUM_NOTES = 5;
const uint16_t TZ_NOTE_DURATION = 300;
const uint16_t TZ_ECHO_DELAY = 150;        // Haunting echo effect

// === SUPERMAN Fanfare (1978) ===
// John Williams' heroic ascending theme opening
const uint8_t SUPERMAN_NOTES[] = {60, 67, 72, 79, 84}; // C-G-C-G-C (ascending octaves)
const uint8_t SUPERMAN_NUM_NOTES = 5;
const uint16_t SUPERMAN_NOTE_DURATION = 500;  // Bold, confident timing
const uint16_t SUPERMAN_PAUSE = 50;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Iconic Movie Sounds Example ===");
  Serial.println("VS1053_MIDI Library - Classic Film Themes");
  Serial.println();
  Serial.println("Featuring legendary motifs from:");
  Serial.println("🦈 Jaws (1975) - The shark's approach");
  Serial.println("🔪 Psycho (1960) - Shower scene terror");  
  Serial.println("🛸 Close Encounters (1977) - Alien communication");
  Serial.println("👁️ Twilight Zone (1959) - Otherworldly mystery");
  Serial.println("🦸 Superman (1978) - Heroic fanfare");
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
  
  // Set volume for dramatic effect
  midi.setMasterVolume(0x25, 0x25);
  delay(100);
  
  Serial.println("Starting movie sound demonstration...");
  Serial.println("Each theme will play with appropriate instrument and timing");
  Serial.println();
  
  delay(2000);
}

void loop() {
  playJaws();
  delay(3000);
  
  playPsycho();
  delay(3000);
  
  playCloseEncounters();
  delay(3000);
  
  playTwilightZone();
  delay(3000);
  
  playSuperman();
  delay(5000);
  
  Serial.println("🎬 Movie sound cycle complete - repeating in 3 seconds...");
  delay(3000);
}

/**
 * JAWS (1975) - The Shark's Approach
 * Two-note motif that builds from slow menace to frantic terror
 */
void playJaws() {
  Serial.println("🦈 JAWS - \"We're gonna need a bigger boat...\"");
  
  // Use Tuba for that low, menacing sound
  midi.setInstrument(MIDI_CHANNEL, GM_Instruments::TUBA);
  
  // Start slow and ominous, build to frantic
  uint16_t tempo = JAWS_SLOW_TEMPO;
  for(int cycle = 0; cycle < 8; cycle++) {
    // Play the two-note pattern
    for(int rep = 0; rep < 4; rep++) {
      midi.noteOn(MIDI_CHANNEL, JAWS_NOTE_1, 100 + (cycle * 5)); // Build volume
      delay(tempo);
      midi.noteOff(MIDI_CHANNEL, JAWS_NOTE_1, 64);
      delay(50);
      
      midi.noteOn(MIDI_CHANNEL, JAWS_NOTE_2, 100 + (cycle * 5));
      delay(tempo);
      midi.noteOff(MIDI_CHANNEL, JAWS_NOTE_2, 64);
      delay(50);
    }
    
    // Accelerate the tempo (shark getting closer!)
    tempo = max(JAWS_FAST_TEMPO, tempo - 80);
    delay(200);
  }
  
  Serial.println("  🩸 *CHOMP* 🦈");
  Serial.println();
}

/**
 * PSYCHO (1960) - Shower Scene Terror
 * Bernard Herrmann's piercing violin stabs
 */
void playPsycho() {
  Serial.println("🔪 PSYCHO - \"*SCREECH* *SCREECH* *SCREECH*\"");
  
  // Use Violin for those piercing high notes
  midi.setInstrument(MIDI_CHANNEL, GM_Instruments::VIOLIN);
  
  // Sudden violent stabs - random pattern for chaos
  for(int burst = 0; burst < 3; burst++) {
    for(int stab = 0; stab < 6; stab++) {
      uint8_t note = PSYCHO_NOTES[random(PSYCHO_NUM_NOTES)];
      midi.noteOn(MIDI_CHANNEL, note, 127); // Maximum violence!
      delay(PSYCHO_STAB_DURATION);
      midi.noteOff(MIDI_CHANNEL, note, 64);
      delay(PSYCHO_PAUSE);
    }
    delay(300); // Brief pause between bursts
  }
  
  Serial.println("  😱 \"Mother!\" 🔪");
  Serial.println();
}

/**
 * CLOSE ENCOUNTERS (1977) - Alien Communication
 * The famous 5-note sequence that bridges human and alien communication
 */
void playCloseEncounters() {
  Serial.println("🛸 CLOSE ENCOUNTERS - \"This means something...\"");
  
  // Use Marimba for that ethereal, precise sound
  midi.setInstrument(MIDI_CHANNEL, GM_Instruments::MARIMBA);
  
  // Play the sequence three times like in the movie
  for(int transmission = 0; transmission < 3; transmission++) {
    Serial.print("  Transmission ");
    Serial.print(transmission + 1);
    Serial.print(": ");
    
    for(int i = 0; i < CE_NUM_NOTES; i++) {
      midi.noteOn(MIDI_CHANNEL, CE_NOTES[i], 90);
      delay(CE_NOTE_DURATION);
      midi.noteOff(MIDI_CHANNEL, CE_NOTES[i], 64);
      
      // Print the note pattern
      const char* noteNames[] = {"D", "E", "C", "C(low)", "G"};
      Serial.print(noteNames[i]);
      if(i < CE_NUM_NOTES - 1) Serial.print("-");
      
      if(i < CE_NUM_NOTES - 1) delay(CE_NOTE_PAUSE);
    }
    Serial.println();
    delay(800);
  }
  
  Serial.println("  👽 \"Welcome to Earth\" 🛸");
  Serial.println();
}

/**
 * TWILIGHT ZONE (1959) - Otherworldly Mystery
 * The eerie descending theme that signals something strange
 */
void playTwilightZone() {
  Serial.println("👁️ TWILIGHT ZONE - \"Do do do do, do do do do...\"");
  
  // Use Theremin-like sound (Flute with effects)
  midi.setInstrument(MIDI_CHANNEL, GM_Instruments::FLUTE);
  
  // Play the descending pattern with eerie echo
  for(int echo = 0; echo < 2; echo++) {
    for(int i = 0; i < TZ_NUM_NOTES; i++) {
      midi.noteOn(MIDI_CHANNEL, TZ_NOTES[i], 80 - (echo * 20)); // Softer echo
      delay(TZ_NOTE_DURATION);
      midi.noteOff(MIDI_CHANNEL, TZ_NOTES[i], 64);
      delay(TZ_ECHO_DELAY);
    }
    delay(500);
  }
  
  Serial.println("  🌀 \"You're traveling through another dimension...\" 👁️");
  Serial.println();
}

/**
 * SUPERMAN (1978) - Heroic Fanfare
 * John Williams' triumphant opening that screams "HERO!"
 */
void playSuperman() {
  Serial.println("🦸 SUPERMAN - \"Look! Up in the sky!\"");
  
  // Use Trumpet for that heroic brass sound
  midi.setInstrument(MIDI_CHANNEL, GM_Instruments::TRUMPET);
  
  // Bold, ascending fanfare
  Serial.print("  Fanfare: ");
  for(int i = 0; i < SUPERMAN_NUM_NOTES; i++) {
    midi.noteOn(MIDI_CHANNEL, SUPERMAN_NOTES[i], 120); // Bold and confident
    delay(SUPERMAN_NOTE_DURATION);
    midi.noteOff(MIDI_CHANNEL, SUPERMAN_NOTES[i], 64);
    
    // Show the ascending pattern
    Serial.print("♪");
    if(i < SUPERMAN_NUM_NOTES - 1) {
      Serial.print("-");
      delay(SUPERMAN_PAUSE);
    }
  }
  Serial.println();
  
  Serial.println("  💪 \"It's a bird... it's a plane... it's Superman!\" 🦸‍♂️");
  Serial.println();
}
