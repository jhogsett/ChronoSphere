NOTES: Fixing rotating displays

SECTIONS:

* Facts
* Current Displays
* Display Problems
* New Displays
* Q and A

FACTS:

* For the HT16K33 display custom library, the decimal point does not consume a segment position. The built-in decimal point of the display segment is used instead without advancing to the next segment position
* The HT16K33 display consists of 12 segments arranged as three groups of four segments. The left-most group is Green; the middle group is Amber; the right-most group is Red.
* For the purposes of the below examples, the following values will be used
    * Real Temperature: 82.1
    * Feels-like Temperature: 83.1
    * Time (AM or PM doesn't matter): 3:07 
    * Date: 03/19
    * Temperature Word: WARM
    * Pressure: 1013
    * Humidity: 51%
    * Light Lux: 110
    * Day of the week: THU
    * The RTC unit specifies day of the week using 0 for Sunday, 1 for Monday and so on for each subsequent day
* In the below examples, a tilde character ~ will be used to indiate un unfilled position in the display. Also the entire sequence for each will be within double-quotes.

CURRENT DISPLAYS:

First - Time and actual temperature: "~30782.1TEMP~"

Second - Date and feels-like temperature: "03/1983.1Feel~"

Third - Temperature word and humidity: "WARM~~~~~83.1"

Fourth - Pressure: "1013PRESHPA~"

Fifth - Light level: "~110LITELUX~"

DISPLAY PROBLEMS:

First - Time and actual temperature: 
* The word TEMP is not needed
* Also it is not positioned entirely within the red display group because the fact that the decimal point does not take up a segment position was not accounted for.

Second - Date and feels-like temperature:
* The Date required five sements, meaning it partiall exists in the amber display group.
* The word FEEL is not needed
* The day of the week is not shown

Third - Temperature word and humidity:
* The feels-like word is always displayed in the green display group instead of taking advantage of the different colored display groups for emphasis

Fourth - Pressure: "1013PRESHPA~"
* The words PRES and HPA are not needed

Fifth - Light level: "~110LITELUX~"
* The words LITE and LUX are not both needed

NEW DISPLAYS:

First - Time, Date and Day of the week: "~30703.19~THU"
* The time is displayed entire within the left-most green display group
* The date is displayed enirely within the middle amber display group
* The date is displayed using a decimal point rather than a slash to separate date and month so that the date always fits within four segments
* The day of the week is displayed, using the right 3 sements of the right-most red display group using standard three-letter abbeviations 
    * 0=SUN, 1=MON, 2=TUE, 3=WED, 4=THU, 5=FRI, 6=SAT

Second - Feels-like temperature word and feels-like temperature: "WARM~~~~~83.1"
* The feels-like temperature word is placed with either the green, amber or red display group according to the following:
    * FROZ, COLD, CHLY, TOSY, HOT: red display group
    * COOL, COZY: amber display group
    * NICE, WARM: green display group
* The feels-like temperate is displayed in another display group based on where the word is placed according to the following:
    * Word in Green display group: feels-like temperature in red display group
    * Word in Amber display group: feels-like temperature in green display group
    * Word in Red display group: feels-like temperature in green display group

Third - Real temperature and humidity: "~82.1~~~~~51%"
    * Real temperature in green display group
    * Humidity in red display group

Fourth - Pressure in millibars and inches mercury: "Pres101329.92"
    * The word "Pres" (maintaining this casing) in the green display group
    * The pressure in millibars in the amber display group
    * The pressure in inches mercury in the red display group

Fifth - Light Level: "Lux~~~~~~110"
    * The word "Lux" left-justified in the green display group
    * The light lux level right-justifierd in a combined 8 segment amber-and-red display group (to allow for a 5 digit value; though usually it will dislay only in the red group)

Q and A:

Question:
% character (Third Display): Standard 7-segment displays can't render a % symbol. Does the HT16K33 library support a custom character for it, or is there an alternate representation planned (e.g., Pct, PCT, or just the numeric value without a unit)?

Answer: 
The HT16K33 displays use 14-segment digits, not 7-segment digits and can display the entire ASCII character set.

Question:
Time separator (First Display): The example ~307 has no visual separator between hour and minutes. Is this intentional — relying on the user knowing green = time — or should there be a dot/decimal between them (e.g., ~3.07)?

Answer:
Yes. This will distinguish it from the Date which uses a decimal point to separate day and month

Question:
"Pres" and "Lux" lowercase letters: 7-segment displays have limited lowercase support. Specifically, lowercase r, e (in "Pres") and u, x (in "Lux") can be tricky. Has the character rendering for these been verified against the HT16K33 library, or does it need to be checked?

Answer: 
The HT16K33 displays use 14-segment digits, not 7-segment digits and can display the entire ASCII character set.

Question:
Second Display — empty amber group: When the feels-like word is in green (WARM, NICE), the temperature goes to red, leaving the entire amber group blank (~~~~). Is this the intended appearance, or would it be preferable to center or shift the content?

Answer:
Yes this is intentional and necessary to keep the subparts of the display within the colored display groups.

Question:
Pressure conversion (Fourth Display): Is the mb → inHg conversion (1013 mb → 29.92 inHg) already computed somewhere in the codebase, or does it need to be added?

Answer:
As far as I am aware the codebase does not currently have code to compute this conversion. I presume it is easy to determine what is needed to do the conversion.

Question:
Negative temperatures: For the Third Display real temperature, would a negative value like -8.2 still fit in the green group as ~-8.2 (4 segment positions with the minus sign)? Or is a different layout needed for sub-zero values?

Answer:
Disregard the possibility of negative temperatures. The units will remain in California.
