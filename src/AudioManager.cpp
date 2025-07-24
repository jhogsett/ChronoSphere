#include <Arduino.h>
#include <SPI.h>
#include "AudioManager.h"

// Constructor - initialize VS1053_MIDI with pin definitions
AudioManager::AudioManager() : musicPlayer(VS1053_CS, VS1053_DCS, VS1053_DREQ, VS1053_RESET) {
}

// Westminster Chime sequences - full 5 changes in E major
// Using MIDI note numbers: G#4=68, F#4=66, E4=64, B3=59
// Based on the authentic Big Ben sequence as documented at Westminster Palace

// Change 1: G#4, F#4, E4, B3 (1st quarter) - kept for backward compatibility
const ChimeNote AudioManager::westminsterChange1[] = {
  {68, 1}, {66, 1}, {64, 1}, {59, 2}  // G#, F#, E, B
};

// Change 2: E4, G#4, F#4, B3 (unused - kept for future expansion)
const ChimeNote AudioManager::westminsterChange2[] = {
  {64, 1}, {68, 1}, {66, 1}, {59, 2}  // E, G#, F#, B
};

// Change 3: E4, F#4, G#4, E4 (unused - kept for future expansion) 
const ChimeNote AudioManager::westminsterChange3[] = {
  {64, 1}, {66, 1}, {68, 1}, {64, 2}  // E, F#, G#, E
};

// Change 4: G#4, E4, F#4, B3 (3rd quarter - USED in hour chime)
const ChimeNote AudioManager::westminsterChange4[] = {
  {68, 1}, {64, 1}, {66, 1}, {59, 2}  // G#, E, F#, B
};

// Change 5: B3, F#4, G#4, E4 (4th quarter - USED in hour chime)
const ChimeNote AudioManager::westminsterChange5[] = {
  {59, 1}, {66, 1}, {68, 1}, {64, 2}  // B, F#, G#, E
};

// Legacy Westminster chime for backward compatibility (same as Change 1)
const ChimeNote AudioManager::westminsterChime[] = {
  {68, 1}, {66, 1}, {64, 1}, {59, 2}  // Same as Change 1
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
  
  // Set default settings
  currentChimeType = CHIME_WESTMINSTER;
  currentInstrument = INSTRUMENT_TUBULAR_BELLS;
  chimeFrequency = 2; // Half-hourly (includes hour and half-hour chimes)
  
  isPlaying = false;
  playStartTime = 0;
  
  // Set master volume (exactly like working example)
  musicPlayer.setMasterVolume(0x20, 0x20);
  
  // Set instrument on channel 0 (exactly like working example)
  musicPlayer.setInstrument(0, currentInstrument);
  delay(100);
  
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
    // Quarter-hourly (every 15 minutes: 0, 15, 30, 45)
    shouldChime = true;
  } else if (chimeFrequency >= 2 && currentMinute % 30 == 0) {
    // Half-hourly (every 30 minutes: 0, 30)
    shouldChime = true;
  } else if (chimeFrequency >= 1 && currentMinute == 0) {
    // Hourly only (at 0 minutes)
    shouldChime = true;
  }
  
  // Only chime if we haven't already chimed this minute
  if (shouldChime && (currentMinute != lastChimeMinute || currentHour != lastChimeHour)) {
    if (currentMinute == 0) {
      // Full hour - play enhanced Westminster if enabled (3rd & 4th quarters + hour strikes)
      if (currentChimeType == CHIME_WESTMINSTER) {
        playFullWestminsterHour(currentHour);
      } else {
        playHourChime(currentHour);
      }
    } else if (currentMinute == 30) {
      // Half hour - play single chime bell
      playHalfHourChime();
    } else if (currentMinute == 15 || currentMinute == 45) {
      // Quarter hours (15, 45 min) - play regular quarter chime
      playTestChime();
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
  
  // Call VS1053 directly to avoid isPlaying conflict
  for (uint8_t i = 0; i < length; i++) {
    musicPlayer.noteOn(0, sequence[i].note, 127);
    delay(sequence[i].duration * 250); // Quarter note = 250ms
    musicPlayer.noteOff(0, sequence[i].note, 127);
    if (i < length - 1) delay(100); // Short pause between notes
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
  
  // Strike the hour - call VS1053 directly to avoid isPlaying conflicts
  for (uint8_t i = 0; i < strikes; i++) {
    musicPlayer.noteOn(0, 72, 127); // High C
    delay(2 * 250); // Half note duration
    musicPlayer.noteOff(0, 72, 127);
    if (i < strikes - 1) {
      delay(500); // Pause between strikes
    }
  }
}

void AudioManager::playHalfHourChime() {
  // Play single chime bell on half hour (traditionally the hour bell)
  if (isPlaying) return;
  
  isPlaying = true;
  playStartTime = millis();
  
  // Play single hour bell note - call VS1053 directly to avoid isPlaying conflict
  if (currentChimeType == CHIME_WESTMINSTER) {
    musicPlayer.noteOn(0, 64, 127);  // E4
    delay(4 * 250); // Whole note duration
    musicPlayer.noteOff(0, 64, 127);
  } else {
    musicPlayer.noteOn(0, 72, 127);  // High C
    delay(4 * 250); // Whole note duration  
    musicPlayer.noteOff(0, 72, 127);
  }
  
  isPlaying = false;
}

void AudioManager::playFullWestminsterHour(uint8_t hour) {
  // Play 3rd and 4th quarter Westminster sequences before hour strikes
  // This plays Changes 4 and 5 only (as requested)
  if (isPlaying) return;
  
  isPlaying = true;
  playStartTime = millis();
  
  // Play Change 4: G#4, E4, F#4, B3 (3rd quarter) - call directly, not through playChimeSequence
  for (uint8_t i = 0; i < 4; i++) {
    musicPlayer.noteOn(0, westminsterChange4[i].note, 127);
    delay(westminsterChange4[i].duration * 250); // Quarter note = 250ms
    musicPlayer.noteOff(0, westminsterChange4[i].note, 127);
    if (i < 3) delay(100); // Short pause between notes
  }
  delay(500); // Pause between 3rd and 4th quarters
  
  // Play Change 5: B3, F#4, G#4, E4 (4th quarter) - call directly, not through playChimeSequence
  for (uint8_t i = 0; i < 4; i++) {
    musicPlayer.noteOn(0, westminsterChange5[i].note, 127);
    delay(westminsterChange5[i].duration * 250); // Quarter note = 250ms
    musicPlayer.noteOff(0, westminsterChange5[i].note, 127);
    if (i < 3) delay(100); // Short pause between notes
  }
  
  delay(1000); // Longer pause before hour strikes
  
  // Convert to 12-hour format and strike the hour
  uint8_t strikes = hour % 12;
  if (strikes == 0) strikes = 12;
  
  // Strike the hour on deeper Big Ben note (A3 = 57 for deeper, more resonant tone)
  // Using same tubular bells instrument but much lower pitch for Big Ben effect
  for (uint8_t i = 0; i < strikes; i++) {
    musicPlayer.noteOn(0, 57, 127);  // A3 - deeper than the Westminster quarters
    delay(4 * 250); // Whole note duration
    musicPlayer.noteOff(0, 57, 127);
    if (i < strikes - 1) {
      delay(1000); // 1 second pause between hour strikes
    }
  }
  
  isPlaying = false;
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

void AudioManager::playStartupChime(uint8_t hour) {
  // Play startup chime to test audio system - Westminster quarters + hour strikes
  
  if (currentChimeType == CHIME_WESTMINSTER) {
    // Play Westminster quarters followed by hour strikes for testing
    if (isPlaying) return;
    
    isPlaying = true;
    playStartTime = millis();
    
    // Play Change 4: G#4, E4, F#4, B3 (3rd quarter)
    for (uint8_t i = 0; i < 4; i++) {
      musicPlayer.noteOn(0, westminsterChange4[i].note, 127);
      delay(westminsterChange4[i].duration * 250); // Quarter note = 250ms
      musicPlayer.noteOff(0, westminsterChange4[i].note, 127);
      if (i < 3) delay(100); // Short pause between notes
    }
    delay(500); // Pause between 3rd and 4th quarters
    
    // Play Change 5: B3, F#4, G#4, E4 (4th quarter)
    for (uint8_t i = 0; i < 4; i++) {
      musicPlayer.noteOn(0, westminsterChange5[i].note, 127);
      delay(westminsterChange5[i].duration * 250); // Quarter note = 250ms
      musicPlayer.noteOff(0, westminsterChange5[i].note, 127);
      if (i < 3) delay(100); // Short pause between notes
    }
    
    // If hour is provided (not 0), play hour strikes
    if (hour > 0) {
      delay(1000); // Longer pause before hour strikes
      
      uint8_t strikes = hour % 12;
      if (strikes == 0) strikes = 12; // Convert 0 to 12 for 12-hour format
      
      // Strike the hour on deeper Big Ben note (A3 = 57 for deeper tone)
      for (uint8_t i = 0; i < strikes; i++) {
        musicPlayer.noteOn(0, 57, 127);  // A3 - deeper than Westminster quarters
        delay(4 * 250); // Whole note duration
        musicPlayer.noteOff(0, 57, 127);
        if (i < strikes - 1) {
          delay(1000); // 1 second pause between hour strikes
        }
      }
    }
    
    isPlaying = false;
  } else {
    // For other chime types, just play the basic chime sequence
    playTestChime();
  }
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
  
  // Play descending alert tone - call VS1053 directly to avoid isPlaying conflict
  for (int note = 80; note >= 60; note -= 4) {
    musicPlayer.noteOn(0, note, 100);
    delay(250); // Quarter note duration
    musicPlayer.noteOff(0, note, 100);
    delay(50); // Short pause between notes
  }
  
  isPlaying = false;
}

void AudioManager::playTemperatureAlert() {
  if (isPlaying) return;
  
  isPlaying = true;
  playStartTime = millis();
  
  // Play ascending temperature alert - call VS1053 directly to avoid isPlaying conflict
  musicPlayer.noteOn(0, 60, 100);
  delay(250); // Quarter note
  musicPlayer.noteOff(0, 60, 100);
  delay(50);
  
  musicPlayer.noteOn(0, 64, 100);
  delay(250); // Quarter note
  musicPlayer.noteOff(0, 64, 100);
  delay(50);
  
  musicPlayer.noteOn(0, 67, 100);
  delay(500); // Half note
  musicPlayer.noteOff(0, 67, 100);
  
  isPlaying = false;
}

void AudioManager::playPressureAlert() {
  if (isPlaying) return;
  
  isPlaying = true;
  playStartTime = millis();
  
  // Play alternating pressure alert - call VS1053 directly to avoid isPlaying conflict
  musicPlayer.noteOn(0, 72, 100);
  delay(250); // Quarter note
  musicPlayer.noteOff(0, 72, 100);
  delay(50);
  
  musicPlayer.noteOn(0, 60, 100);
  delay(250); // Quarter note
  musicPlayer.noteOff(0, 60, 100);
  delay(50);
  
  musicPlayer.noteOn(0, 72, 100);
  delay(250); // Quarter note
  musicPlayer.noteOff(0, 72, 100);
  
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
