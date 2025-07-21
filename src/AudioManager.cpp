#include <Arduino.h>
#include "AudioManager.h"

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
  // if (!musicPlayer.begin()) {  // Replace with your custom VS1053 init
  //   Serial.println(F("VS1053 initialization failed"));
  //   return false;
  // }
  
  // Set default settings
  currentChimeType = CHIME_WESTMINSTER;
  currentInstrument = INSTRUMENT_TUBULAR_BELLS;
  chimeFrequency = 1; // Hourly only
  
  isPlaying = false;
  playStartTime = 0;
  
  // Set volume (0-100)
  // musicPlayer.setVolume(50, 50);  // Replace with your custom VS1053 volume control
  
  // Set MIDI mode and select instrument
  // musicPlayer.switchToMidiMode();  // Replace with your custom VS1053 MIDI setup
  // musicPlayer.midiSetChannelBank(0, VS1053_BANK_MELODY);
  // musicPlayer.midiSetInstrument(0, currentInstrument);
  
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
  
  int currentMinute = currentTime.minute();
  int currentHour = currentTime.hour();
  
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
  
  // musicPlayer.midiNoteOn(0, note, velocity);   // Replace with your custom VS1053 note on
  delay(duration * 250); // Quarter note = 250ms
  // musicPlayer.midiNoteOff(0, note, velocity);  // Replace with your custom VS1053 note off
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
  // musicPlayer.midiSetInstrument(0, instrument);  // Replace with your custom VS1053 instrument selection
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
  // for (int i = 0; i < 128; i++) {
  //   musicPlayer.midiNoteOff(0, i, 0);  // Replace with your custom VS1053 note off
  // }
  isPlaying = false;
}

bool AudioManager::isBusy() {
  return isPlaying;
}

void AudioManager::setVolume(uint8_t volume) {
  // musicPlayer.setVolume(volume, volume);  // Replace with your custom VS1053 volume control
}
