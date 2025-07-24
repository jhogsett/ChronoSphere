#ifndef CONFIG_H
#define CONFIG_H

// Pin Definitions
#define ROTARY_CLK_PIN 2
#define ROTARY_DT_PIN 3
#define ROTARY_SW_PIN 4
#define STEPPER_PIN1 14
#define STEPPER_PIN2 15
#define STEPPER_PIN3 16
#define STEPPER_PIN4 17
#define SERVO_PIN 5
#define NEOPIXEL_PIN 6
#define VS1053_CS 10    // Command interface
#define VS1053_DCS 9    // Data interface
#define VS1053_RESET 8  // Reset pin
#define VS1053_DREQ 7   // Data request pin
// IC2 reserved A4 A5
// SPI reserved 13 12 11
#define NEOPIXEL_COUNT 8  // Reduced from 12 to 8 to save RAM

// I2C Addresses
#define RTC_ADDRESS 0x68
#define AHT21_ADDRESS 0x38
// #define BMP280_ADDRESS 0x76  // Commented out to avoid conflict with library
#define BH1750_ADDRESS 0x23
#define DISPLAY_GREEN_ADDRESS 0x70
#define DISPLAY_AMBER_ADDRESS 0x71
#define DISPLAY_RED_ADDRESS 0x72

// Display Brightness Compensation (compensates for LED color variations)
#define DISPLAY_GREEN_BRIGHTNESS 1   // Green LEDs are very bright
#define DISPLAY_AMBER_BRIGHTNESS 9   // Amber LEDs are medium brightness
#define DISPLAY_RED_BRIGHTNESS 15    // Red LEDs are dim

// Timing Constants
#define STEPPER_STEPS_PER_HOUR 2048
#define PENDULUM_SWING_PERIOD 2000  // milliseconds
#define SENSOR_READ_INTERVAL 30000   // 30 seconds
#define DISPLAY_UPDATE_INTERVAL 1000 // 1 second
#define CHIME_CHECK_INTERVAL 60000   // 1 minute

// Temperature Ranges for Four-Letter Words (Fahrenheit)
#define TEMP_FROZ_MAX 19    // FROZ: <= 19°F
#define TEMP_COLD_MAX 34    // COLD: 20-34°F
#define TEMP_CHLY_MAX 49    // CHLY (Chilly): 35-49°F
#define TEMP_COOL_MAX 64    // COOL: 50-64°F
#define TEMP_NICE_MAX 79    // NICE: 65-79°F
#define TEMP_WARM_MAX 84    // WARM: 80-84°F
#define TEMP_COZY_MAX 89    // COZY: 85-89°F
#define TEMP_TOSY_MAX 94    // TOSY (Toasty): 90-94°F
#define TEMP_HOT_MAX 104    // HOT: 95-104°F
                            // SCOR (Scorching): >= 105°F

// Display Color Zones
#define COMFORT_GREEN_MIN 65  // NICE range start
#define COMFORT_GREEN_MAX 79  // NICE range end
#define COMFORT_RED_MIN 20    // COLD range start
#define COMFORT_RED_MAX 34    // COLD range end, also HOT+ ranges

// Data Storage - Optimized for Arduino Nano memory constraints
#define MAX_HOURLY_RECORDS 24    // 24 hours of hourly data
#define MAX_DAILY_RECORDS 7      // 7 days of daily data
#define EEPROM_DATA_START 0

// Chime Types
enum ChimeType {
  CHIME_WESTMINSTER = 0,
  CHIME_WHITTINGTON,
  CHIME_ST_MICHAELS,
  CHIME_CUSTOM
};

// MIDI Instruments
enum MidiInstrument {
  INSTRUMENT_TUBULAR_BELLS = 14,
  INSTRUMENT_VIBRAPHONE = 11,
  INSTRUMENT_GLOCKENSPIEL = 9,
  INSTRUMENT_CHURCH_ORGAN = 19
};

// Display Modes
enum DisplayMode {
  MODE_CLOCK = 0,
  MODE_TEMPERATURE,
  MODE_WEATHER_SUMMARY,
  MODE_ROLLING_CURRENT,
  MODE_ROLLING_HISTORICAL,
  MODE_ROLLING_TRENDS,
  MODE_SETTINGS
};

// Settings Menu Items
enum SettingItem {
  SETTING_TIME = 0,
  SETTING_DATE,
  SETTING_CHIME_TYPE,
  SETTING_CHIME_INSTRUMENT,
  SETTING_CHIME_FREQUENCY,
  SETTING_SENSOR_SENSITIVITY,
  SETTING_WEATHER_ALERTS,
  SETTING_LIGHTING_EFFECTS,
  SETTING_MOTOR_SETTINGS,
  SETTING_EXIT
};

// Alert Types
enum AlertType {
  ALERT_NONE = 0,
  ALERT_PRESSURE,
  ALERT_TEMPERATURE,
  ALERT_RAPID_CHANGE
};

// Alert States
enum AlertState {
  ALERT_STATE_IDLE = 0,
  ALERT_STATE_FLASH_ON,
  ALERT_STATE_FLASH_OFF
};

#endif
