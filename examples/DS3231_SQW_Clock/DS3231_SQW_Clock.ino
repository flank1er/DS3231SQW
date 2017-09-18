/*
 * simple example for receiving current date from RTC
 * and subsequent calculation date on  microcontroller side
 * through SQW/OUT pin DS3231
 * work only with DS3231
 * connect DS1307 to Arduino
 * Analog pin 5 - SCL
 * Analog pin 4 - SDA
 * Digital pin 2 - SQW/OUT
 * Connect for MSP-EXP430G2 Launchpad with MSP430G2553
 * Pin 14 - SCL
 * Pin 15 - SDA
 * Pin 11 - SQW
 * Connect for STM32duino with stm32f103c8t6 - Blue_Pill
 * PB6 - SCL
 * PB7 - SDA
 * PA0 - SQW
 */
#include "Wire.h"
#include "RTC_DS3231.h"

#ifdef __MSP430__
#define LED RED_LED
#define SQW 11
#elif __AVR__
#define LED 13
#define SQW 2
#elif __thumb__
#define LED PC13
#define SQW PA0
#endif

DS3231 rtc;

void handler();
volatile int state = LOW;

void setup()
{
	Serial.begin(9600);
	Wire.begin();

	pinMode(LED, OUTPUT);
	digitalWrite(LED, state);
	pinMode(SQW, INPUT_PULLUP);
#ifdef __AVR__
	attachInterrupt(digitalPinToInterrupt(SQW), handler, FALLING);
#elif __MSP430__ || __thumb__
  attachInterrupt(SQW, handler, FALLING);
#endif
	// RTC Setup ////////
	rtc.force_update(); // get date from RTC
	rtc.set_self_update(ENABLE); // enable calculation date on  microcontroller side
	rtc.set_sqw(DS3231_1HZ); // enable calculation date through external interrupt, 1sec=1tick
	rtc.reset_control(DS3231_INTCH); // enable SQW mode for INT/SQW pin DS3231
}

void loop()
{
	rtc.print_calendar();
	state=!state;
	digitalWrite(LED, state);
	delay(1000);
}

void handler() {
	rtc++;
}

