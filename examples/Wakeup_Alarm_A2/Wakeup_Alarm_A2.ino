/*
 * simple example for using alarm A2 DS3231 
 * for wakeup microcontroller from sleep mode
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
#include <avr/sleep.h>
#define LED 13
#define SQW 2
#elif __thumb__
#include <libmaple/pwr.h>
#include <libmaple/scb.h>
// These are possibly defined somewhere but I couldn't find them. System Control Register
#define SCB_SCR_SLEEPDEEP 3       // Controls deepsleep(1) or sleep(0)
#define SCB_SCR_SLEEPONEXIT 2     // Controls sleeponexit (not used here)
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
	rtc.force_update();
	rtc.set_self_update(ENABLE);
  rtc.preset_control(DS3231_INTCH);
}

void loop()
{
  static int i=0;
	rtc.print_calendar();
	if (rtc.is_alarm(ALARM_A2))
      Serial.println("Alarm 2 is ON");
    else
      Serial.println("Alarm 2 is OFF");

    if (rtc.is_alarm(ALARM_A1))
      Serial.println("Alarm 1 is ON");
    else
      Serial.println("Alarm 1 is OFF");

  if (++i==5) {
    Serial.println();
    Serial.println("Set alarm A2...");
    rtc.set_alarm_a2(A2_MATCH_MINUTES, (rtc.get(MINUTE)+2)%60, 0,0);
    rtc.print_alarm_2();
    delay(1000);
#ifdef __AVR__    
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu();
#elif __MSP430__
    _BIS_SR(LPM3_bits + GIE); // Enter LPM3 w/interrupt
#elif __thumb__
    // Clear PDDS and LPDS bits
    PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS;
    // set sleepdeep in the system control register
    SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;
    asm("wfi");
#endif
    delay(1000);
    rtc.disable_alarm(ALARM_A2);
    rtc.force_update();
  }
 
	state=!state;
	digitalWrite(LED, state);
	delay(1000);
}

void handler() {
	Serial.println("WAKEUP!");
}
