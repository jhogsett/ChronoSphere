#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "Sensors.h"  // This includes DS3231-RTC.h with DateTime class
#include "VS1053_MIDI.h"  // VS1053 MIDI library
#include "Config.h"

struct ChimeNote {
  uint8_t note;
  uint8_t duration;  // in quarter notes
};

class AudioManager {
private:
  VS1053_MIDI musicPlayer;  // VS1053 MIDI object (will be initialized in constructor)
  
  ChimeType currentChimeType;
  MidiInstrument currentInstrument;
  uint8_t chimeFrequency;  // 1=hourly, 2=half-hourly, 4=quarter-hourly
  
  bool isPlaying;
  unsigned long playStartTime;
  
  // Chime sequences
  static const ChimeNote westminsterChime[];
  static const ChimeNote whittingtonChime[];
  static const ChimeNote stMichaelsChime[];
  
  void playChimeSequence(const ChimeNote* sequence, uint8_t length);
  void playHourChime(uint8_t hour);

public:
  // Constructor
  AudioManager();
  
  bool init();
  void update();
  
  // Core playback functions
  void playNote(uint8_t note, uint8_t velocity, uint16_t duration);
  
  // Chime functions
  void checkAndPlayChime(DateTime currentTime);
  void playTestChime();
  void setChimeType(ChimeType type);
  void setChimeInstrument(MidiInstrument instrument);
  void setChimeFrequency(uint8_t frequency);
  
  // Alert functions
  void playWeatherAlert();
  void playTemperatureAlert();
  void playPressureAlert();
  
  // Settings
  ChimeType getChimeType();
  MidiInstrument getChimeInstrument();
  uint8_t getChimeFrequency();
  
  // Control functions
  void stopPlaying();
  bool isBusy();
  void setVolume(uint8_t volume);
};

#endif
