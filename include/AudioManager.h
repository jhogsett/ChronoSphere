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
  // Westminster enhancement: Play 3rd & 4th quarters before hour chimes
  // Chime frequency settings:
  // - 1: Hourly only (0 min) - Hour: Changes 4,5 + strikes | Half/Quarters: None
  // - 2: Half-hourly (0,30 min) - Hour: Changes 4,5 + strikes | Half: Single bell
  // - 4: Quarter-hourly (0,15,30,45 min) - All above + Quarters: Regular chime
  static const ChimeNote westminsterChime[];
  static const ChimeNote westminsterChange1[];
  static const ChimeNote westminsterChange2[];
  static const ChimeNote westminsterChange3[];
  static const ChimeNote westminsterChange4[];
  static const ChimeNote westminsterChange5[];
  static const ChimeNote whittingtonChime[];
  static const ChimeNote stMichaelsChime[];
  
  void playChimeSequence(const ChimeNote* sequence, uint8_t length);
  void playHourChime(uint8_t hour);
  void playHalfHourChime();
  void playFullWestminsterHour(uint8_t hour);

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
  void playStartupChime(); // Play startup chime for main app initialization
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
