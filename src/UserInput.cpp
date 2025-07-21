#include <Arduino.h>
#include "UserInput.h"

UserInput::UserInput() : rotaryEncoder(ROTARY_CLK_PIN, ROTARY_DT_PIN) {
  lastEncoderPosition = 0;
  currentEncoderPosition = 0;
  buttonPin = ROTARY_SW_PIN;
  buttonState = BUTTON_IDLE;
  buttonPressTime = 0;
  lastButtonCheck = 0;
  lastButtonReading = HIGH;
}

bool UserInput::init() {
  // Initialize button pin
  pinMode(buttonPin, INPUT_PULLUP);
  
  // Initialize encoder
  rotaryEncoder.write(0);
  lastEncoderPosition = 0;
  currentEncoderPosition = 0;
  
  Serial.println(F("User input initialized"));
  return true;
}

void UserInput::update() {
  unsigned long currentTime = millis();
  
  // Update encoder reading
  currentEncoderPosition = rotaryEncoder.read() / 4; // Divide by 4 for cleaner steps
  
  // Update button state with debouncing
  if (currentTime - lastButtonCheck > DEBOUNCE_DELAY) {
    bool currentReading = digitalRead(buttonPin) == LOW; // Active low
    
    if (currentReading && !lastButtonReading) {
      // Button just pressed
      buttonState = BUTTON_PRESSED;
      buttonPressTime = currentTime;
    } else if (!currentReading && lastButtonReading) {
      // Button just released
      buttonState = BUTTON_RELEASED;
    } else if (currentReading && lastButtonReading) {
      // Button held down
      if (currentTime - buttonPressTime > LONG_PRESS_TIME) {
        buttonState = BUTTON_HELD;
      }
    } else {
      // Button not pressed
      buttonState = BUTTON_IDLE;
    }
    
    lastButtonReading = currentReading;
    lastButtonCheck = currentTime;
  }
}

int UserInput::getEncoderDelta() {
  int delta = currentEncoderPosition - lastEncoderPosition;
  lastEncoderPosition = currentEncoderPosition;
  return delta;
}

int UserInput::getEncoderPosition() {
  return currentEncoderPosition;
}

void UserInput::resetEncoderPosition() {
  rotaryEncoder.write(0);
  lastEncoderPosition = 0;
  currentEncoderPosition = 0;
}

ButtonState UserInput::getButtonState() {
  return buttonState;
}

bool UserInput::isButtonPressed() {
  return buttonState == BUTTON_PRESSED;
}

bool UserInput::isButtonHeld() {
  return buttonState == BUTTON_HELD;
}

bool UserInput::wasButtonReleased() {
  return buttonState == BUTTON_RELEASED;
}

DisplayMode UserInput::handleModeChange(DisplayMode currentMode, int encoderDelta) {
  if (encoderDelta == 0) return currentMode;
  
  int modeInt = (int)currentMode;
  
  if (encoderDelta > 0) {
    modeInt++;
    if (modeInt > MODE_SETTINGS) {
      modeInt = MODE_CLOCK;
    }
  } else {
    modeInt--;
    if (modeInt < MODE_CLOCK) {
      modeInt = MODE_SETTINGS;
    }
  }
  
  return (DisplayMode)modeInt;
}

SettingItem UserInput::handleSettingChange(SettingItem currentSetting, int encoderDelta) {
  if (encoderDelta == 0) return currentSetting;
  
  int settingInt = (int)currentSetting;
  
  if (encoderDelta > 0) {
    settingInt++;
    if (settingInt > SETTING_MOTOR_SETTINGS) {
      settingInt = SETTING_TIME;
    }
  } else {
    settingInt--;
    if (settingInt < SETTING_TIME) {
      settingInt = SETTING_MOTOR_SETTINGS;
    }
  }
  
  return (SettingItem)settingInt;
}
