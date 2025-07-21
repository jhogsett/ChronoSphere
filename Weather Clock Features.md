## Weather Clock Features

### Hardware

- Based on Arduino Nano / Nano Every
- Has Sensors
    - DS3231 Real Time Clock Module (I2C)
    - AHT21 Digital Temperature and Humidity Sensor (I2C)
    - BMP280BMP280 Digital Barometric Pressure Temperature Sensor (I2C)
    - BH1750 Light Sensor (I2C)
- Has Interface Hardware
    - 12-Character Display
        - Based on HT16K33 (I2C)
        - Consists of 3 four-character modules (Green, Amber, Red)
    - Rotary encoder switch with pushbutton 
        - for modes and settings
    - Generic VS1053 Module for MIDI sounds
        - for clock chime
        - for weather alerts
    - WS-2812 Addressable LEDs for lighting effects
- Has Functional Hardware
    - 28BYJ48 4-Phase Stepper Motor with 2048 steps for an animated Minute Hand
    - Standard 5V Servo for an animated Pendulum

### Software

#### Top Level Features

- Operates as a clock
    - Displays the time
    - Displays the date
    - Shows the current minute using the stepper motor
    - Swings the pendulum using the servo at a pleasing rate
    - Plays an hourly clock chime
- Display basic data collected from on-board sensors
    - Displays the temperature
    - Displays the humidity
    - Displays the barometric pressure
    - Displays the light level
- Records historical sensor data to enable advanced reporting
    - Displays averaged sensor data based periods such as last hour, last day, last week, last month
    - Displays minimum and maximum sensor data levels reached during the averaged periods
    - Keeps recent data necessary to detect trends and perform basic automatic weather prediction
- Allows the user to choose operating modes and set preferences using the rotary encoder with pushbutton and display
    - Example modes (set by turning the rotary knob):
        - Show the temperature
        - Show temperature, humidity and pressure
        - Show a rolling billboard of current data
        - Show a rolling billboard of historical data
        - Show a rolling billboard of trend and prediction data
        - Show various combinations of the above
    - Example Settings (set by pressing the rotary knob first then setting):
        - Set the time and date
        - Which hourly chime to play, for example, Westminster, Whittington, St. Michael's etc.
        - Which General Midi instrument to use for the chime, for example, Tubular Bells, Vibraphone, Glockenspiel
        - Chime Frequency, for example, Hourly only, also bell every 30 minutes, also bell every 15 minutes
        - Possible settings needed for sensor sensitivity
        - Possible settings needed for weather prediction 
        - Settings for lighting effects to be determined
        - Settings for alerts, for example, sudden drop in pressure in increase in temperature
        - Settings for stepper motor and server to be determined

## Special Temperature Features

- Temperature is displayed in three different ways
    - As the raw value in degrees Fahrenheit with one decimal, such as 104.5
    - As the computed "feels like" in degrees Fahrenheit with integers only, such as 88
        - The "feels like" value is computed based on NOAA weather standards (and we have working example code for this)
    - As a four-letter word representing the "feels like" value such as "NICE", "COZY", "COOL"
        - Each word represents a 5-degree temperature range 
    - The display template is changed based on whether the "feels like" temperature is "comfortable", "uncomfortable" or in-between
        - The most comfortable temperatures display the four-letter word on the green display
        - The least comforable temperatures display the four-letter word on the red display
        - The remaining temperatures display the four-letter word on the amber display

## About the Light Sensor

- The light sensor is not necessarily involved with weather prediction or other clock-like features
    - There may be interested uses for it, such as detecting human presence

## About the Stepper Motor

- The Stepper Motor will be used to show an animated minute hand, supplementing the other time reporting features
    - it will be attached to a dial with a minute hand connected to the shaft
    - it has 2048 steps, and should step evenly once per each 1/2048th of an hour
        - 2048 doesn't divide into even seconds
        - This should be accounted for in the math to ensure it revolves exactly one time each hour

## About the Servo

- The Servo will be used to create a to-be-determined pendulum 
- It will need asynchronous stepping code that can smoothly change the position with a step function in a main event loop without blocking

## About the VS1053 Midi Sounds Generator

- This will be used to play notes on instruments for the hourly chime and other sounds
- The user will be able to choose from multiple hourly chime types
- Certain other events maybe cause a chime to be played, as as a reaching a new high "feels like" temperature four-letter word for the day
- It may also be used to play alarms, such as if there is a sudden acute trend in weather

## Possible Derivative data

- Could display the sunrise and sunset time, and chime when they occur
    - would need the installed latitude/longitude and some sophisticated math

