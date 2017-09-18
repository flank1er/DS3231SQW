/*
 * example code illustrating DS3231SQW library set current date through serial port messages.
 * Messages consist of the letter T followed by ten digit time (as seconds since Jan 1 1970)
 * you can send the text on the next line using Serial Monitor to set the clock to noon Jan 1 2017  
 * T1357387200
 *
 * A Processing example sketch to automatically send the messages is included in the download
 * On Linux, you can use "TZ_adjust=3; echo T$(($(date +%s)+60*60*$TZ_adjust)) > /dev/ttyACM0" (MSK time zone)
 * 
 * connect for Arduino
 * Analog pin 5 - SCL ;
 * Analog pin 4 - SDA ;
 * connect for MSP-EXP430G2 Launchpad with MSP430G2553
 * Pin 14 - SCL
 * Pin 15 - SDA
 * Connect for STM32duino with stm32f103c8t6 - Blue_Pill
 * PB6 - SCL
 * PB7 - SDA
 * 
 * For Arduino Nano, in order to avoid a reset, it is required to install a capacitor 
 * at 10μF on pins Reset(anode) and Ground(Cathode).
 */
#include "Wire.h"
#include "RTC_DS1307.h"

#define TIME_HEADER  "T"

#ifdef __MSP430__
#define LED RED_LED
#elif __AVR__
#define LED 13
#elif __thumb__
#define LED PC13
#endif

DS1307 rtc;

void processSyncMessage();
volatile int state = LOW;

void setup()
{
	Serial.begin(9600);
	Wire.begin();

	pinMode(LED, OUTPUT);
	digitalWrite(LED, state);
	// RTC Setup ////////
	rtc.force_update();
	rtc.set_self_update(DISABLE);

	Serial.println("Waiting for sync message");
}

void loop()
{
	if (Serial.available())
		processSyncMessage();

	rtc.print_calendar();
	state=!state;
	digitalWrite(LED, state);
	delay(1000);
}

void processSyncMessage() {
	unsigned long pctime;
	const unsigned long DEFAULT_TIME = 1357387200; // Jan 1 2017

	if(Serial.find(TIME_HEADER))
	{
		pctime = Serial.parseInt();
		if( pctime > DEFAULT_TIME) // check the integer is a valid time (greater than Jan 1 2017)
			rtc.set_date(pctime); // Sync Arduino clock to the time received on the serial port
	}
}

