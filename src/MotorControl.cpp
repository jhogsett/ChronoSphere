#include <Arduino.h>
#include "MotorControl.h"

// Step sequence for 28BYJ48 stepper motor
const int MotorControl::stepSequence[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

bool MotorControl::init() {
  // Initialize stepper motor pins
  stepperPins[0] = STEPPER_PIN1;
  stepperPins[1] = STEPPER_PIN2;
  stepperPins[2] = STEPPER_PIN3;
  stepperPins[3] = STEPPER_PIN4;
  
  for (int i = 0; i < 4; i++) {
    pinMode(stepperPins[i], OUTPUT);
    digitalWrite(stepperPins[i], LOW);
  }
  
  // Initialize servo
  pendulumServo.attach(SERVO_PIN);
  pendulumServo.write(90); // Center position
  
  // Initialize variables
  currentStepperPosition = 0;
  lastStepperUpdate = 0;
  stepperInterval = 1000; // Default interval
  currentStep = 0;
  
  pendulumPosition = 90;
  pendulumDirection = 1;
  lastPendulumUpdate = 0;
  pendulumInterval = PENDULUM_SWING_PERIOD / 2;
  pendulumEnabled = true;
  
  // Serial.println(F("Motor control initialized"));
  return true;
}

void MotorControl::update() {
  unsigned long currentTime = millis();
  
  // Update stepper motor if it's time
  if (currentTime - lastStepperUpdate >= stepperInterval) {
    // Stepper updates are handled by updateMinuteHand()
    lastStepperUpdate = currentTime;
  }
  
  // Update pendulum
  updatePendulum();
}

void MotorControl::updateMinuteHand(int currentMinute, int currentSecond) {
  // Calculate target position based on time
  // The stepper has 2048 steps per full rotation (one hour)
  float minutesElapsed = currentMinute + (currentSecond / 60.0);
  int targetPosition = (int)(minutesElapsed * STEPPER_STEPS_PER_HOUR / 60.0);
  
  // Move stepper toward target position
  if (currentStepperPosition != targetPosition) {
    bool forward = targetPosition > currentStepperPosition;
    stepMotor(forward);
    
    if (forward) {
      currentStepperPosition++;
      if (currentStepperPosition >= STEPPER_STEPS_PER_HOUR) {
        currentStepperPosition = 0;
      }
    } else {
      currentStepperPosition--;
      if (currentStepperPosition < 0) {
        currentStepperPosition = STEPPER_STEPS_PER_HOUR - 1;
      }
    }
  }
  
  // Calculate timing for smooth movement
  calculateStepperTiming(currentMinute, currentSecond);
}

void MotorControl::stepMotor(bool forward) {
  if (forward) {
    currentStep = (currentStep + 1) % 8;
  } else {
    currentStep = (currentStep - 1 + 8) % 8;
  }
  
  // Apply step pattern to pins
  for (int i = 0; i < 4; i++) {
    digitalWrite(stepperPins[i], stepSequence[currentStep][i]);
  }
  
  delay(2); // Small delay for stepper timing
  
  // Turn off all pins to save power (optional)
  // for (int i = 0; i < 4; i++) {
  //   digitalWrite(stepperPins[i], LOW);
  // }
}

void MotorControl::calculateStepperTiming(int currentMinute, int currentSecond) {
  // Calculate how many milliseconds per step for smooth movement
  // We want to complete 2048 steps in 3600 seconds (1 hour)
  // But we need to account for the fact that 2048 doesn't divide evenly into 3600
  
  float millisecondsPerStep = 3600000.0 / STEPPER_STEPS_PER_HOUR; // ~1758.8 ms per step
  stepperInterval = (unsigned long)millisecondsPerStep;
  
  // Fine-tune timing to ensure we complete exactly one rotation per hour
  // This compensates for the non-even division
  float adjustment = (currentSecond % 60) * 0.1; // Small adjustment based on seconds
  stepperInterval += (long)adjustment;
}

void MotorControl::updatePendulum() {
  if (!pendulumEnabled) return;
  
  unsigned long currentTime = millis();
  
  if (currentTime - lastPendulumUpdate >= pendulumInterval) {
    // Update pendulum position
    pendulumPosition += pendulumDirection * 5; // Move 5 degrees per update
    
    // Reverse direction at limits
    if (pendulumPosition >= 135) {
      pendulumPosition = 135;
      pendulumDirection = -1;
    } else if (pendulumPosition <= 45) {
      pendulumPosition = 45;
      pendulumDirection = 1;
    }
    
    // Apply position to servo
    pendulumServo.write(pendulumPosition);
    
    lastPendulumUpdate = currentTime;
  }
}

void MotorControl::calibrateMinuteHand() {
  // Serial.println(F("Calibrating minute hand..."));
  
  // Move to known position (12 o'clock)
  for (int i = 0; i < STEPPER_STEPS_PER_HOUR / 4; i++) {
    stepMotor(true);
    delay(5);
  }
  
  currentStepperPosition = 0;
  // Serial.println(F("Minute hand calibrated"));
}

void MotorControl::calibratePendulum() {
  // Serial.println(F("Calibrating pendulum..."));
  
  // Center pendulum
  pendulumServo.write(90);
  pendulumPosition = 90;
  pendulumDirection = 1;
  
  delay(500);
  // Serial.println(F("Pendulum calibrated"));
}

void MotorControl::setStepperSpeed(float stepsPerSecond) {
  if (stepsPerSecond > 0) {
    stepperInterval = (unsigned long)(1000.0 / stepsPerSecond);
  }
}

void MotorControl::setPendulumEnabled(bool enabled) {
  pendulumEnabled = enabled;
  if (!enabled) {
    pendulumServo.write(90); // Center position when disabled
  }
}

void MotorControl::setPendulumSpeed(unsigned long period) {
  pendulumInterval = period / 2; // Half period since we update twice per swing
}

int MotorControl::getStepperPosition() {
  return currentStepperPosition;
}

int MotorControl::getPendulumPosition() {
  return pendulumPosition;
}

void MotorControl::emergencyStop() {
  // Turn off all stepper pins
  for (int i = 0; i < 4; i++) {
    digitalWrite(stepperPins[i], LOW);
  }
  
  // Center pendulum
  pendulumServo.write(90);
  pendulumEnabled = false;
  
  // Serial.println(F("Emergency stop activated"));
}
