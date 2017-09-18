/*
 * simple example for receiving current date from RTC
 * and subsequent calculation date on  microcontroller side.
 * work with DS1307 and DS3231
 * connect for Arduino
 * Analog pin 5 - SCL ;
 * Analog pin 4 - SDA ;
 * connect for MSP-EXP430G2 Launchpad with MSP430G2553
 * Pin 14 - SCL
 * Pin 15 - SDA
 * Connect for STM32duino with stm32f103c8t6 - Blue_Pill
 * PB6 - SCL
 * PB7 - SDA
 */
#include "Wire.h"
#include "RTC_DS1307.h"

#ifdef __MSP430__
#define LED RED_LED
#elif __AVR__
#define LED 13
#elif __thumb__
#define LED PC13
#endif

DS1307 rtc;
volatile int state = LOW;

void setup()
{
	Serial.begin(9600);
	Wire.begin();

	pinMode(LED, OUTPUT);
	digitalWrite(LED, state);

	// RTC Setup ////////
	rtc.force_update();   //  receive current date from RTC
	rtc.set_self_update(ENABLE); // enable calculation date on  microcontroller side
}

void loop()
{
	rtc.print_calendar();
	state=!state;
	digitalWrite(LED, state);
	delay(1000);
}
