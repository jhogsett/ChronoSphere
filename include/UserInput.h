#ifndef USER_INPUT_H
#define USER_INPUT_H

#include <Encoder.h>
#include "Config.h"

enum ButtonState {
  BUTTON_IDLE = 0,
  BUTTON_PRESSED,
  BUTTON_HELD,
  BUTTON_RELEASED
};

class UserInput {
private:
  Encoder rotaryEncoder;
  
  int lastEncoderPosition;
  int currentEncoderPosition;
  
  uint8_t buttonPin;
  ButtonState buttonState;
  unsigned long buttonPressTime;
  unsigned long lastButtonCheck;
  bool lastButtonReading;
  
  // Debouncing
  static const unsigned long DEBOUNCE_DELAY = 50;
  static const unsigned long LONG_PRESS_TIME = 1000;

public:
  UserInput();
  bool init();
  void update();
  
  // Encoder functions
  int getEncoderDelta();
  int getEncoderPosition();
  void resetEncoderPosition();
  
  // Button functions
  ButtonState getButtonState();
  bool isButtonPressed();
  bool isButtonHeld();
  bool wasButtonReleased();
  
  // Mode switching
  DisplayMode handleModeChange(DisplayMode currentMode, int encoderDelta);
  SettingItem handleSettingChange(SettingItem currentSetting, int encoderDelta);
};

#endif
