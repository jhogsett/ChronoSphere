Questions and Updates March 26, 2026

SECTIONS:
* Questions
* Updates
* Rules
* Testing Follow-Up

QUESTIONS:

1. The original hybrid clock code that was derived-from to create the code we're using here has a recalibration of the minute arm every four hours to ensure it maintains positional accuracy. Is that four-hour recalibration working in the present set of code?

2. The original hybrid clock code also has a feature to show the clock LEDs less brightly during certain hours of the nighttime. Is that dimmin in the present set of code?

3. Is the HT16K33 display set updated continuously, or only when a change is needed? My impression is it is updated continuously. If so, that may not be necessary and possibly lead to errors due to a lot of I2C communication.

4. Is the "feels like" temperature computed according to the NOAA method of computing it, given temperature and humidity?


UPDATES:

1. On startup, the Hybrid clock initialization happens causing the minute arm calibration, then the muinute arm moves to the proper minute position, after which the ChronoSphere identification is shown on the HT16K33 display set, but without the clock lighting up the hour and background pattern LEDs. This causes a clumsy appearance on start up.

Do this:
    * Show the Chronosphere identication first before the hybrid clock initialization
    * Then initilaize the hybrid clock, leaving it updated so that the hour and background pattern LEDs are displayed.

2. On the half-hour, the clock chimes once time to indicate it's the half-hour point in time. However, it sounds like that chime is not the same instrument and/or note as the chiming done on the top of the hour, which seems inconsistent.

Do this:
    * Make sure the half-hour single chime uses the same instrument and note as the one through 12 chimes that occur on the top of the hour.

3. The device seems to show the "temp alert" every 10 minutes and does not seem to settle and eventually stop showing the "temp alert" as if there is some problem with the eeprom historical data.

Do this:
    * When the device starts up, the historical data should be set to the currently read values so that the "temp alert" (and other weather alerts) are always based on current data, not older out of date data.

4. If there is an initialization failure on start up, it just shows "ERR INIT FAIL" but does not indicate why the initialzaiton failed.

Do this:
    * When the initialization fails, show what caused the failure
    * What's shown should start with "F " and use the remaining 10 chars to show the causes, so that it fits within the 12-segment HT16K33 display set.


RULES:

1. Address the questions first before working on the updates.

2. When the updates are done, they need to be done one-by-one, with testing on the device to confirm the device still works prior to moving on to the next update.


TESTING FOLLOW-UP:

1. The startup sequence is now:
    * Show "ChronoSphere" identification
    * 2-second delay
    * Hybrid clock initialization (but still no showing of the hour and background pattern LEDs)
    * Playing the current hourly chime (purpose is to allow setting analog volume level)
    * Showing the hybrid clock hour and background pattern LEDs and normal device operation

Do this:
    * Remove the 2-second delay (because the hybrid clock initialization creates enough of a delay to see the ChronoSphere identifcation)
    * After the hybrid clock initialization, allow the clock to update normally so it shows the hour and background pattern LEDs (otherwise the start up sequence still seems clumsy)
    * Playing the current hourly chime
    * Normal device operation from then on
    