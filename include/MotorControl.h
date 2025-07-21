#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Servo.h>
#include "Config.h"

class MotorControl {
private:
  // Stepper motor for minute hand
  uint8_t stepperPins[4];
  int currentStepperPosition;
  unsigned long lastStepperUpdate;
  unsigned long stepperInterval;
  
  // Servo for pendulum
  Servo pendulumServo;
  int pendulumPosition;
  int pendulumDirection;
  unsigned long lastPendulumUpdate;
  unsigned long pendulumInterval;
  bool pendulumEnabled;
  
  // Stepper motor step patterns
  static const int stepSequence[8][4];
  int currentStep;
  
  void stepMotor(bool forward);
  void calculateStepperTiming(int currentMinute, int currentSecond);

public:
  bool init();
  void update();
  
  // Stepper motor functions
  void updateMinuteHand(int currentMinute, int currentSecond);
  void calibrateMinuteHand();
  void setStepperSpeed(float stepsPerSecond);
  
  // Servo functions
  void updatePendulum();
  void setPendulumEnabled(bool enabled);
  void setPendulumSpeed(unsigned long period);
  void calibratePendulum();
  
  // Utility functions
  int getStepperPosition();
  int getPendulumPosition();
  void emergencyStop();
};

#endif
