Encoder Changes

SECTIONS
* Facts
* Current Interface
* New Interface


FACTS
* In the below description, unless otherwise mentioned, the encoder will be turned clockwise (a natural forward movement to a human user)
* For the HT16K33 display custom library, the decimal point does not consume a segment position. The built-in decimal point of the display segment is used instead without advancing to the next segment position
* The HT16K33 display consists of 12 segments arranged as three groups of four segments. The left-most group is Green; the middle group is Amber; the right-most group is Red.
* For the purposes of the below examples, the following values will be used
    * Real Temperature: 82.1
    * Feels-like Temperature: 83.1
    * Time (AM or PM doesn't matter): 307 
    * Date: 03.19
    * Temperature Word: WARM
    * Pressure (millibars): 1013
    * Pressure (inches mercuty): 29.92
    * Humidity: 51%
    * Light Lux: 110
    * Day of the week: THU
* In the below examples, a tilde character ~ will be used to indiate un unfilled position in the display. Also the entire sequence for each will be within double-quotes.
* IMPORTANT NOTE: the data and clock pins of the rotary encoder are currently backwards as of the writing of this document, meaning that a counter-clockwise rotation is seen in the code as the forward movement. This will be changed by the user ahead of the agent-made changes.

CURRENT INTERFACE
* On start up, the display defaults to the rotating displays (good, keep this)
* When turning the encoder clockwise, the following displays are seen:
    1. Temperature, humidity and pressure in the format "82.1~51%1010~"
    2. Temperature, feels-like temperature, feels-like temperature word in the format "82.1~83.1NICE~"
    3. Time and Date in the format "~307~~~3~19~"
    4. Settings mode in the format "Settings~~~~"
    5. Trends mode in the format "Trends~~~~~~"
    6. History mode in the format "History~~~~~"
    7. Rotating display mdoe (the default on startup) in several formats.

* When Settings is shown, pressing the encoder switch goes into Settings Mode and the following display is seen:
    * "Set~TIME~~~~"
    * When turning the encoder clockwise, the following display are seen:
        1. "EXIT~~~~~~~~"
        2. "Setting~008~"
        3. "Setting~007~"
        4. "Setting~006~"
        5. "Setting~005~"
        6. "Chime~FREQ~~"
        7. "Chime~INSTR~"
        8. "Chime~TYPE~~"
        9. "Set~DATE~~~~"
        10. (Back to) "Set~TIME~~~~"
    * Important Note: Settings 005 through 008 are stubbed for now and will remain stubbed after the changes made from this document (but will be handled at a later time).

Important note from the agent:

One code naming issue worth knowing for the New Interface section:
The mode names MODE_TEMPERATURE and MODE_WEATHER_SUMMARY are essentially swapped relative to what they display:

MODE_TEMPERATURE (item 2 in your list) actually shows temp + feels-like + word — no humidity or pressure
MODE_WEATHER_SUMMARY (item 1) actually shows temp + humidity + pressure
