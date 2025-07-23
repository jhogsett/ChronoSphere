// now.pde
// Prints a snapshot of the current date and time along with the UNIX time
// Modified by Andy Wickert from the JeeLabs / Ladyada RTC library examples
// 5/15/11

#include <Wire.h>
#include <DS3231-RTC.h>
#include <HT16K33Disp.h>

// RTClib myRTC;

DS3231 myRTC;

bool century = false;
bool h12Flag;
bool pm;

HT16K33Disp disp1(0x70, 3);
// HT16K33Disp disp2(0x71, 1);
// HT16K33Disp disp3(0x72, 1);

#define DISPLAY_BRIGHTNESS 2

void setup () {
    // Serial.begin(57600);
    Wire.begin();


  byte brightness1[3] = { DISPLAY_BRIGHTNESS, DISPLAY_BRIGHTNESS, DISPLAY_BRIGHTNESS };
//   byte brightness2[1] = { 2 };
//   byte brightness3[1] = { 2 };


  disp1.init(brightness1);
//   disp2.init(brightness2);
//   disp3.init(brightness3);

    // myRTC.setClockMode(false);  // set to 24h
    // myRTC.setClockMode(true); // set to 12h
}

#define DATETIME_CUTOFF 57

bool start = true;    
int last_second = -1;
void loop () {

    int second = myRTC.getSecond();
    if(second == last_second)
        return;
    last_second = second;

    int year = myRTC.getYear();
    int month = myRTC.getMonth(century);
    int day = myRTC.getDate();
    int hour = myRTC.getHour(h12Flag, pm);
    int minute = myRTC.getMinute();

    char date[20];
    char time[20];

    sprintf_P(date, PSTR(" %2d/%02d/20%01d"), month, day, year);
    if(pm){
        sprintf_P(time, PSTR("  %2d. %02d %02d"), hour, minute, second);
    } else {
        sprintf_P(time, PSTR("  %2d %02d %02d"), hour, minute, second);
    }

    if(start){
        start = false;
        char buffer[40];
        sprintf_P(buffer, PSTR("%s%s"), date, time);
        disp1.scroll_string(buffer);
    }

    if(second >= DATETIME_CUTOFF){
        disp1.show_string(date);
    } else if(second < DATETIME_CUTOFF){
        disp1.show_string(time);
    }

    // Serial.println(minute);    
    // Serial.println(date);
    // Serial.println(time);

    // delay(1000);
}
