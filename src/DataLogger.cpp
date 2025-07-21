#include <Arduino.h>
#include "DataLogger.h"
#include <string.h>  // For memset

bool DataLogger::init() {
  currentHourlyIndex = 0;
  currentDailyIndex = 0;
  currentSampleIndex = 0;
  lastLogTime = 0;
  
  // Initialize arrays
  memset(hourlyData, 0, sizeof(hourlyData));
  memset(dailyData, 0, sizeof(dailyData));
  memset(currentHourSamples, 0, sizeof(currentHourSamples));
  
  // Load existing data from EEPROM
  loadFromEEPROM();
  
  Serial.println(F("Data logger initialized"));
  return true;
}

void DataLogger::update(SensorData currentData) {
  unsigned long currentTime = millis();
  
  // Store current sample
  currentHourSamples[currentSampleIndex] = currentData;
  currentSampleIndex = (currentSampleIndex + 1) % 12;  // Reduced from 60 to 12
  
  // Check if it's time to log hourly data
  DateTime now = currentData.currentTime;
  if (now.hour() != lastHourLogged.hour() || 
      (lastHourLogged.year() == 0)) { // First run
    logHourlyData();
    lastHourLogged = now;
  }
  
  // Check if it's time to log daily data
  if (now.day() != lastDayLogged.day() || 
      (lastDayLogged.year() == 0)) { // First run
    logDailyData();
    lastDayLogged = now;
  }
  
  lastLogTime = currentTime;
}

void DataLogger::logHourlyData() {
  if (currentSampleIndex == 0) return; // No samples yet
  
  HourlyRecord record;
  record.timestamp = currentHourSamples[0].currentTime;
  
  // Calculate averages
  float tempSum = 0, humSum = 0, pressSum = 0;
  float minTemp = 999, maxTemp = -999;
  float minPress = 9999, maxPress = 0;
  
  uint8_t validSamples = 0;
  for (int i = 0; i < currentSampleIndex && i < 12; i++) {  // Limited to 12 samples
    if (currentHourSamples[i].currentTime.year() > 0) { // Valid sample
      tempSum += currentHourSamples[i].temperatureF;
      humSum += currentHourSamples[i].humidity;
      pressSum += currentHourSamples[i].pressure;
      
      if (currentHourSamples[i].temperatureF < minTemp) {
        minTemp = currentHourSamples[i].temperatureF;
      }
      if (currentHourSamples[i].temperatureF > maxTemp) {
        maxTemp = currentHourSamples[i].temperatureF;
      }
      if (currentHourSamples[i].pressure < minPress) {
        minPress = currentHourSamples[i].pressure;
      }
      if (currentHourSamples[i].pressure > maxPress) {
        maxPress = currentHourSamples[i].pressure;
      }
      
      validSamples++;
    }
  }
  
  if (validSamples > 0) {
    record.avgTemperature = tempSum / validSamples;
    record.avgHumidity = humSum / validSamples;
    record.avgPressure = pressSum / validSamples;
    record.minTemperature = minTemp;
    record.maxTemperature = maxTemp;
    record.minPressure = minPress;
    record.maxPressure = maxPress;
    
    // Store in circular buffer
    hourlyData[currentHourlyIndex] = record;
    currentHourlyIndex = (currentHourlyIndex + 1) % MAX_HOURLY_RECORDS;
    
    // Save to EEPROM
    saveToEEPROM();
    
    Serial.println(F("Hourly data logged"));
  }
  
  // Reset sample buffer
  currentSampleIndex = 0;
}

void DataLogger::logDailyData() {
  // Calculate daily averages from hourly data
  DailyRecord record;
  record.date = lastDayLogged;
  
  float tempSum = 0, humSum = 0, pressSum = 0;
  float minTemp = 999, maxTemp = -999;
  float minPress = 9999, maxPress = 0;
  int validHours = 0;
  
  // Look at last 24 hours of data
  for (int i = 0; i < 24 && i < MAX_HOURLY_RECORDS; i++) {
    int index = (currentHourlyIndex - 1 - i + MAX_HOURLY_RECORDS) % MAX_HOURLY_RECORDS;
    HourlyRecord &hourly = hourlyData[index];
    
    if (hourly.timestamp.year() > 0) { // Valid record
      tempSum += hourly.avgTemperature;
      humSum += hourly.avgHumidity;
      pressSum += hourly.avgPressure;
      
      if (hourly.minTemperature < minTemp) minTemp = hourly.minTemperature;
      if (hourly.maxTemperature > maxTemp) maxTemp = hourly.maxTemperature;
      if (hourly.minPressure < minPress) minPress = hourly.minPressure;
      if (hourly.maxPressure > maxPress) maxPress = hourly.maxPressure;
      
      validHours++;
    }
  }
  
  if (validHours > 0) {
    record.avgTemperature = tempSum / validHours;
    record.avgHumidity = humSum / validHours;
    record.avgPressure = pressSum / validHours;
    record.minTemperature = minTemp;
    record.maxTemperature = maxTemp;
    record.minPressure = minPress;
    record.maxPressure = maxPress;
    
    // Store in circular buffer
    dailyData[currentDailyIndex] = record;
    currentDailyIndex = (currentDailyIndex + 1) % MAX_DAILY_RECORDS;
    
    Serial.println(F("Daily data logged"));
  }
}

HourlyRecord DataLogger::getHourlyRecord(uint8_t hoursAgo) {
  if (hoursAgo >= MAX_HOURLY_RECORDS) {
    return HourlyRecord(); // Return empty record
  }
  
  int index = (currentHourlyIndex - 1 - hoursAgo + MAX_HOURLY_RECORDS) % MAX_HOURLY_RECORDS;
  return hourlyData[index];
}

DailyRecord DataLogger::getDailyRecord(uint8_t daysAgo) {
  if (daysAgo >= MAX_DAILY_RECORDS) {
    return DailyRecord(); // Return empty record
  }
  
  int index = (currentDailyIndex - 1 - daysAgo + MAX_DAILY_RECORDS) % MAX_DAILY_RECORDS;
  return dailyData[index];
}

float DataLogger::getAverageTemperature(uint8_t hours) {
  float sum = 0;
  int count = 0;
  
  for (uint8_t i = 0; i < hours && i < MAX_HOURLY_RECORDS; i++) {
    HourlyRecord record = getHourlyRecord(i);
    if (record.timestamp.year() > 0) {
      sum += record.avgTemperature;
      count++;
    }
  }
  
  return count > 0 ? sum / count : 0;
}

float DataLogger::getAveragePressure(uint8_t hours) {
  float sum = 0;
  int count = 0;
  
  for (uint8_t i = 0; i < hours && i < MAX_HOURLY_RECORDS; i++) {
    HourlyRecord record = getHourlyRecord(i);
    if (record.timestamp.year() > 0) {
      sum += record.avgPressure;
      count++;
    }
  }
  
  return count > 0 ? sum / count : 0;
}

TrendData DataLogger::calculateTrends() {
  TrendData trends;
  
  // Get current and 3-hour-ago data for trend calculation
  HourlyRecord current = getHourlyRecord(0);
  HourlyRecord threeHoursAgo = getHourlyRecord(3);
  
  if (current.timestamp.year() > 0 && threeHoursAgo.timestamp.year() > 0) {
    // Calculate trends per hour
    trends.temperatureTrend = (current.avgTemperature - threeHoursAgo.avgTemperature) / 3.0;
    trends.pressureTrend = (current.avgPressure - threeHoursAgo.avgPressure) / 3.0;
    trends.humidityTrend = (current.avgHumidity - threeHoursAgo.avgHumidity) / 3.0;
    
    // Determine pressure trends
    trends.risingPressure = trends.pressureTrend > 1.0; // Rising > 1 hPa/hour
    trends.fallingPressure = trends.pressureTrend < -1.0; // Falling > 1 hPa/hour
    trends.rapidTempChange = abs(trends.temperatureTrend) > 2.0; // > 2°F/hour
  } else {
    memset(&trends, 0, sizeof(trends));
  }
  
  return trends;
}

bool DataLogger::checkPressureAlert() {
  TrendData trends = calculateTrends();
  return trends.fallingPressure && trends.pressureTrend < -2.0; // Rapid pressure drop
}

bool DataLogger::checkTemperatureAlert() {
  TrendData trends = calculateTrends();
  return trends.rapidTempChange && abs(trends.temperatureTrend) > 5.0; // Very rapid temp change
}

bool DataLogger::checkRapidChange() {
  TrendData trends = calculateTrends();
  return trends.rapidTempChange || abs(trends.pressureTrend) > 3.0;
}

void DataLogger::saveToEEPROM() {
  // Save key data to EEPROM for persistence
  // This is a simplified implementation
  int addr = EEPROM_DATA_START;
  EEPROM.put(addr, currentHourlyIndex);
  addr += sizeof(currentHourlyIndex);
  EEPROM.put(addr, currentDailyIndex);
}

void DataLogger::loadFromEEPROM() {
  // Load key data from EEPROM
  int addr = EEPROM_DATA_START;
  EEPROM.get(addr, currentHourlyIndex);
  addr += sizeof(currentHourlyIndex);
  EEPROM.get(addr, currentDailyIndex);
  
  // Validate loaded values
  if (currentHourlyIndex >= MAX_HOURLY_RECORDS) currentHourlyIndex = 0;
  if (currentDailyIndex >= MAX_DAILY_RECORDS) currentDailyIndex = 0;
}

void DataLogger::clearAllData() {
  memset(hourlyData, 0, sizeof(hourlyData));
  memset(dailyData, 0, sizeof(dailyData));
  currentHourlyIndex = 0;
  currentDailyIndex = 0;
  currentSampleIndex = 0;
  
  Serial.println(F("All data cleared"));
}

bool DataLogger::isDataValid() {
  return currentHourlyIndex > 0 || currentDailyIndex > 0;
}

uint8_t DataLogger::getDataAge() {
  // Return hours since oldest valid data
  HourlyRecord oldest = getHourlyRecord(MAX_HOURLY_RECORDS - 1);
  if (oldest.timestamp.year() > 0) {
    return MAX_HOURLY_RECORDS;
  }
  
  return currentHourlyIndex;
}
