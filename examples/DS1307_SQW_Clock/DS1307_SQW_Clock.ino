/*
 * simple example for receiving current date from RTC
 * and subsequent calculation date on  microcontroller side
 * through SQW/OUT pin DS1307
 * work only with DS1307
 * connect DS1307 to Arduino
 * Analog pin 5 - SCL
 * Analog pin 4 - SDA
 * Digital pin 2 - SQW/OUT
 */
#include "Wire.h"
#include "RTC_DS1307.h"

#define LED 13
#define SQW 2

DS1307 rtc;

void handler();
volatile int state = LOW;

void setup()
{
	Serial.begin(9600);
	Wire.begin();

	pinMode(LED, OUTPUT);
	digitalWrite(LED, state);

	pinMode(SQW, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(SQW), handler, FALLING);

	// RTC Setup ////////
	rtc.force_update(); // get date from RTC
	rtc.set_self_update(ENABLE); // enable calculation date on  microcontroller side
	rtc.set_control(FREQ_1HZ); // enable calculation date through external interrupt, 1sec=1tick
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
