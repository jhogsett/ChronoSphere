#include <Arduino.h>
#include <SPI.h>
#include "AudioManager.h"

// Constructor - initialize VS1053_MIDI with pin definitions
AudioManager::AudioManager() : musicPlayer(VS1053_CS, VS1053_DCS, VS1053_DREQ, VS1053_RESET) {
}

// Westminster Chime sequence (first 4 notes of each quarter)
const ChimeNote AudioManager::westminsterChime[] = {
  {60, 1}, {64, 1}, {67, 1}, {72, 2}  // C, E, G, C (quarter notes)
};

// Whittington Chime sequence
const ChimeNote AudioManager::whittingtonChime[] = {
  {72, 1}, {69, 1}, {65, 1}, {60, 2}  // C, A, F, C
};

// St. Michael's Chime sequence  
const ChimeNote AudioManager::stMichaelsChime[] = {
  {67, 1}, {64, 1}, {60, 1}, {67, 2}  // G, E, C, G
};

bool AudioManager::init() {
  // Initialize SPI first (exactly like working example)
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  
  // Initialize VS1053 MIDI player with MIDI plugin (exactly like working example)
  if (!musicPlayer.begin(true)) {
    Serial.println(F("VS1053 initialization failed"));
    return false;
  }
  
  Serial.println(F("✓ VS1053_MIDI initialized successfully!"));
  Serial.print(F("Plugin size: "));
  Serial.print(musicPlayer.getPluginSize());
  Serial.println(F(" words"));
  
  // Set default settings
  currentChimeType = CHIME_WESTMINSTER;
  currentInstrument = INSTRUMENT_TUBULAR_BELLS;
  chimeFrequency = 1; // Hourly only
  
  isPlaying = false;
  playStartTime = 0;
  
  // Set master volume (exactly like working example)
  musicPlayer.setMasterVolume(0x20, 0x20);
  
  // Set instrument on channel 0 (exactly like working example)
  musicPlayer.setInstrument(0, currentInstrument);
  delay(100);
  
  Serial.println(F("Audio manager initialized"));
  return true;
}

void AudioManager::update() {
  // Check if playback has finished
  if (isPlaying) {
    unsigned long currentTime = millis();
    if (currentTime - playStartTime > 10000) { // 10 second timeout
      isPlaying = false;
    }
  }
}

void AudioManager::checkAndPlayChime(DateTime currentTime) {
  static int lastChimeMinute = -1;
  static int lastChimeHour = -1;
  
  int currentMinute = currentTime.getMinute();
  int currentHour = currentTime.getHour();
  
  bool shouldChime = false;
  
  // Check if it's time to chime based on frequency setting
  if (chimeFrequency >= 4 && currentMinute % 15 == 0) {
    // Quarter-hourly
    shouldChime = true;
  } else if (chimeFrequency >= 2 && currentMinute % 30 == 0) {
    // Half-hourly
    shouldChime = true;
  } else if (chimeFrequency >= 1 && currentMinute == 0) {
    // Hourly
    shouldChime = true;
  }
  
  // Only chime if we haven't already chimed this minute
  if (shouldChime && (currentMinute != lastChimeMinute || currentHour != lastChimeHour)) {
    if (currentMinute == 0) {
      playHourChime(currentHour);
    } else {
      playTestChime(); // Quarter or half-hour chime
    }
    
    lastChimeMinute = currentMinute;
    lastChimeHour = currentHour;
  }
}

void AudioManager::playNote(uint8_t note, uint8_t velocity, uint16_t duration) {
  if (isPlaying) return;
  
  musicPlayer.noteOn(0, note, velocity);
  delay(duration * 250); // Quarter note = 250ms
  musicPlayer.noteOff(0, note, velocity);
}

void AudioManager::playChimeSequence(const ChimeNote* sequence, uint8_t length) {
  if (isPlaying) return;
  
  isPlaying = true;
  playStartTime = millis();
  
  for (uint8_t i = 0; i < length; i++) {
    playNote(sequence[i].note, 127, sequence[i].duration);
    delay(100); // Short pause between notes
  }
  
  isPlaying = false;
}

void AudioManager::playHourChime(uint8_t hour) {
  // Play the quarter chime first
  playTestChime();
  
  delay(500); // Pause between chime and hour strikes
  
  // Convert to 12-hour format
  uint8_t strikes = hour % 12;
  if (strikes == 0) strikes = 12;
  
  // Strike the hour
  for (uint8_t i = 0; i < strikes; i++) {
    playNote(72, 127, 2); // High C, half note
    if (i < strikes - 1) {
      delay(500); // Pause between strikes
    }
  }
}

void AudioManager::playTestChime() {
  const ChimeNote* sequence;
  uint8_t length;
  
  switch (currentChimeType) {
    case CHIME_WESTMINSTER:
      sequence = westminsterChime;
      length = 4;
      break;
    case CHIME_WHITTINGTON:
      sequence = whittingtonChime;
      length = 4;
      break;
    case CHIME_ST_MICHAELS:
      sequence = stMichaelsChime;
      length = 4;
      break;
    default:
      return;
  }
  
  playChimeSequence(sequence, length);
}

void AudioManager::setChimeType(ChimeType type) {
  currentChimeType = type;
}

void AudioManager::setChimeInstrument(MidiInstrument instrument) {
  currentInstrument = instrument;
  musicPlayer.setInstrument(0, instrument);
}

void AudioManager::setChimeFrequency(uint8_t frequency) {
  chimeFrequency = frequency;
}

void AudioManager::playWeatherAlert() {
  if (isPlaying) return;
  
  isPlaying = true;
  playStartTime = millis();
  
  // Play descending alert tone
  for (int note = 80; note >= 60; note -= 4) {
    playNote(note, 100, 1);
  }
  
  isPlaying = false;
}

void AudioManager::playTemperatureAlert() {
  if (isPlaying) return;
  
  isPlaying = true;
  playStartTime = millis();
  
  // Play ascending temperature alert
  playNote(60, 100, 1);
  playNote(64, 100, 1);
  playNote(67, 100, 2);
  
  isPlaying = false;
}

void AudioManager::playPressureAlert() {
  if (isPlaying) return;
  
  isPlaying = true;
  playStartTime = millis();
  
  // Play alternating pressure alert
  playNote(72, 100, 1);
  playNote(60, 100, 1);
  playNote(72, 100, 1);
  
  isPlaying = false;
}

ChimeType AudioManager::getChimeType() {
  return currentChimeType;
}

MidiInstrument AudioManager::getChimeInstrument() {
  return currentInstrument;
}

uint8_t AudioManager::getChimeFrequency() {
  return chimeFrequency;
}

void AudioManager::stopPlaying() {
  // Stop all MIDI notes
  for (int i = 0; i < 128; i++) {
    musicPlayer.noteOff(0, i, 0);
  }
  isPlaying = false;
}

bool AudioManager::isBusy() {
  return isPlaying;
}

void AudioManager::setVolume(uint8_t volume) {
  musicPlayer.setVolume(0, volume);  // Channel 0, volume 0-127
}
