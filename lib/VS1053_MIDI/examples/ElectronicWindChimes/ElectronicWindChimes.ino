/**
 * ElectronicWindChimes - Advanced example for VS1053_MIDI library
 * 
 * This example creates an electronic wind chime system that cycles through
 * different instrument scenarios, playing random notes to simulate wind chimes.
 * Demonstrates advanced MIDI features including:
 * - Multiple instrument scenarios
 * - Random note generation
 * - Variable timing and velocity
 * - Smooth transitions between scenarios
 * 
 * Hardware connections (Arduino Nano):
 * - MISO: Pin 12
 * - MOSI: Pin 11  
 * - SCK:  Pin 13
 * - DREQ: Pin 2
 * - XRST: Pin 8
 * - XCS:  Pin 6
 * - XDCS: Pin 7
 * 
 * Optional: Connect a potentiometer to A0 to control wind intensity
 */

#include <SPI.h>
#include <VS1053_MIDI.h>

// Pin definitions for VS1053 module
#define VS1053_XCS   6    // Chip Select for commands (active low)
#define VS1053_XDCS  7    // Chip Select for data (active low)
#define VS1053_DREQ  2    // Data Request (indicates ready for more data)
#define VS1053_RESET 8    // Reset pin (active low)

// Optional wind sensor pin
#define WIND_SENSOR_PIN A0

// Create VS1053_MIDI instance
VS1053_MIDI midi(VS1053_XCS, VS1053_XDCS, VS1053_DREQ, VS1053_RESET);

// Wind chime scenarios
struct ChimeScenario {
  uint8_t instrument;
  uint8_t baseNote;
  uint8_t noteRange;
  uint16_t minDelay;
  uint16_t maxDelay;
  uint8_t minVelocity;
  uint8_t maxVelocity;
  const char* name;
};

ChimeScenario scenarios[] = {
  {GM_Instruments::TUBULAR_BELLS, 60, 24, 200, 1000, 40, 90, "Tubular Bells"},
  {GM_Instruments::MARIMBA, 72, 20, 150, 800, 50, 95, "Marimba"},
  {GM_Instruments::VIBRAPHONE, 67, 18, 300, 1200, 35, 80, "Vibraphone"},
  {GM_Instruments::CELESTA, 72, 24, 100, 600, 45, 85, "Celesta"}
};

const int numScenarios = sizeof(scenarios) / sizeof(scenarios[0]);
int currentScenario = 0;
unsigned long lastScenarioChange = 0;
const unsigned long scenarioInterval = 20000; // Change scenario every 20 seconds (reduced from 30)

void setup() {
  Serial.begin(115200);
  Serial.println("VS1053 MIDI Library - Electronic Wind Chimes");
  Serial.println("Cycling through different chime scenarios...");
  
  // Initialize random seed
  randomSeed(analogRead(A1)); // Use unused analog pin for noise
  
  // Initialize SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  
  // Initialize VS1053 with MIDI plugin
  if (midi.begin(true)) {
    Serial.println("VS1053 MIDI initialized successfully!");
  } else {
    Serial.println("VS1053 MIDI initialization failed!");
    while(1);
  }
  
  // Set master volume
  midi.setMasterVolume(0x25, 0x25);
  
  // Initialize first scenario
  changeScenario();
}

void loop() {
  // Check if it's time to change scenarios
  if (millis() - lastScenarioChange > scenarioInterval) {
    currentScenario = (currentScenario + 1) % numScenarios;
    changeScenario();
  }
  
  // Get wind intensity (simulated or from sensor)
  int windIntensity = getWindIntensity();
  
  // Play chime notes based on wind intensity
  if (windIntensity > 15) { // Lowered threshold for more activity
    playChimeNote(windIntensity);
  }
  
  // Variable delay based on wind intensity (more wind = more frequent notes)
  // Reduced delay range for more responsive chimes
  int delayTime = map(windIntensity, 0, 100, 1000, 100);
  delay(delayTime);
}

void changeScenario() {
  lastScenarioChange = millis();
  ChimeScenario* scenario = &scenarios[currentScenario];
  
  Serial.print("Switching to scenario: ");
  Serial.println(scenario->name);
  
  // Stop all notes on current channel
  midi.allNotesOff(0);
  delay(100);
  
  // Change to new instrument
  midi.setInstrument(0, scenario->instrument);
  delay(200);
  
  // Play a gentle intro sequence for the new instrument
  playIntroSequence();
}

void playIntroSequence() {
  ChimeScenario* scenario = &scenarios[currentScenario];
  
  // Play 3-4 notes in the instrument's range as an introduction
  for (int i = 0; i < 3; i++) {
    uint8_t note = scenario->baseNote + random(0, scenario->noteRange);
    uint8_t velocity = scenario->minVelocity + random(0, scenario->maxVelocity - scenario->minVelocity);
    
    midi.noteOn(0, note, velocity);
    delay(400);
    midi.noteOff(0, note, 64);
    delay(200);
  }
}

void playChimeNote(int windIntensity) {
  ChimeScenario* scenario = &scenarios[currentScenario];
  
  // Select random note within the scenario's range
  uint8_t note = scenario->baseNote + random(0, scenario->noteRange);
  
  // Calculate velocity based on wind intensity and scenario parameters
  uint8_t velocityRange = scenario->maxVelocity - scenario->minVelocity;
  uint8_t velocity = scenario->minVelocity + (velocityRange * windIntensity / 100);
  
  // Play the note
  midi.noteOn(0, note, velocity);
  
  // Note duration varies with wind intensity
  int duration = map(windIntensity, 0, 100, 200, 800);
  delay(duration);
  
  midi.noteOff(0, note, 64);
  
  // Optional: print what we're playing
  Serial.print("Note: ");
  Serial.print(note);
  Serial.print(", Velocity: ");
  Serial.print(velocity);
  Serial.print(", Wind: ");
  Serial.println(windIntensity);
}

int getWindIntensity() {
  // Check if we have a wind sensor connected
  int sensorValue = analogRead(WIND_SENSOR_PIN);
  
  // If sensor reading is very low (likely no sensor), simulate wind
  if (sensorValue < 10) {
    // Simulate varying wind with some randomness and slow changes
    static float windLevel = 60.0; // Start with higher base wind level
    static unsigned long lastWindUpdate = 0;
    
    if (millis() - lastWindUpdate > 100) {
      // Slowly varying wind with random gusts
      windLevel += random(-3, 4) * 0.8; // More stable variations
      
      // Add occasional gusts
      if (random(100) < 8) { // 8% chance of gust (increased from 5%)
        windLevel += random(15, 35);
      }
      
      // Keep wind level in bounds with a higher minimum
      windLevel = constrain(windLevel, 25, 100); // Minimum wind of 25
      lastWindUpdate = millis();
    }
    
    return (int)windLevel;
  } else {
    // Use actual sensor reading
    return map(sensorValue, 0, 1023, 0, 100);
  }
}

// Additional function to manually trigger different scenarios
void triggerScenario(int scenario) {
  if (scenario >= 0 && scenario < numScenarios) {
    currentScenario = scenario;
    changeScenario();
  }
}

// Function to create harmonic chimes (optional advanced feature)
void playHarmonicChime(int windIntensity) {
  ChimeScenario* scenario = &scenarios[currentScenario];
  
  // Play main note
  uint8_t baseNote = scenario->baseNote + random(0, scenario->noteRange);
  uint8_t velocity = scenario->minVelocity + (windIntensity * (scenario->maxVelocity - scenario->minVelocity) / 100);
  
  midi.noteOn(0, baseNote, velocity);
  
  // Add harmonic if wind is strong enough
  if (windIntensity > 60) {
    uint8_t harmonicNote = baseNote + 7; // Perfect fifth
    if (harmonicNote <= 127) {
      midi.noteOn(0, harmonicNote, velocity * 0.7);
      delay(600);
      midi.noteOff(0, harmonicNote, 64);
    }
  }
  
  delay(400);
  midi.noteOff(0, baseNote, 64);
}
