#ifndef __RTC_DS3231_H__
#define __RTC_DS3231_H__

#include "RTC_DS1307.h"

// control register
#define DS3231_A1IE  0x01
#define DS3231_A2IE  0x02
#define DS3231_INTCH 0x04
#define DS3231_CONV  0x20
#define DS3231_BBSQW 0x40
#define DS3231_EOSC	 0x80

#define DS3231_1HZ 0x0
#define DS3231_1024HZ 0x08
#define DS3231_4096HZ 0x10
#define DS3231_8192HZ 0x18
#define DS3231_32768HZ 0x01

// status register
#define DS3231_A1F	0x01
#define DS3231_A2F	0x02
#define DS3231_BSY	0x04
#define DS3231_32KHZ 0x08
#define DS3231_OSF	 0x80
// Alarm
enum ALARM_NUMBER {ALARM_A1=0x01, ALARM_A2=0x02};

//Alarm masks
enum ALARM_A1_TYPE {
    A1_EVERY_SECOND		= 0x0F,
    A1_MATCH_SECONDS	= 0x0E,
    A1_MATCH_MINUTES	= 0x0C,     //match minutes *and* seconds
    A1_MATCH_HOURS		= 0x08,     //match hours *and* minutes, seconds
    A1_MATCH_DATE		= 0x00,     //match date *and* hours, minutes, seconds
    A1_MATCH_DAY		= 0x10,     //match day *and* hours, minutes, seconds
};

enum ALARM_A2_TYPE {
    A2_EVERY_MINUTE		= 0x8E,
    A2_MATCH_MINUTES	= 0x8C,     //match minutes
    A2_MATCH_HOURS		= 0x88,     //match hours *and* minutes
    A2_MATCH_DATE		= 0x80,     //match date *and* hours, minutes
    A2_MATCH_DAY		= 0x90,     //match day *and* hours, minutes
};



struct ALARM {
	unsigned second_a1 :7;
    unsigned a1m1 : 1;
    unsigned minute_a1 :7;
    unsigned a1m2 : 1;
    unsigned hour_a1 :7;
    unsigned a1m3 : 1;
    unsigned day_date_a1 :6;
	unsigned dydt_a1: 1;
    unsigned a1m4 : 1;

    unsigned minute_a2 :7;
    unsigned a2m2 : 1;
    unsigned hour_a2 :7;
    unsigned a2m3 : 1;
    unsigned day_date_a2 :6;
    unsigned dydt_a2: 1;
    unsigned a2m4 : 1;

	unsigned control :8;
	unsigned status : 8;
	unsigned aging : 8;
};

class DS3231: public DS1307 {
private:
	ALARM alarm;
	char temp_MSB;
	char temp_LSB;
	uint8_t control;
	void update_status_control();
	void print_bcd(uint8_t);
public:
	DS3231();
	char get_temperature();
	char* get_temperature_fraction();
	//
	void set(uint8_t);
	bool is_alarm(ALARM_NUMBER);
	void set_sqw(const uint8_t);
    void force_update();
	// control
	uint8_t get_control();
	void preset_control(uint8_t);
    void set_control(uint8_t);
    void reset_control(uint8_t);
	// status
	uint8_t get_status();
	void set_status(uint8_t);
	void reset_status(uint8_t);
	// aging
	uint8_t get_aging();
	void set_aging(uint8_t);
	// print
	void print_calendar();
	void print_alarm_1();
	void print_alarm_2();
	// alarm
	void set_alarm_a1(ALARM_A1_TYPE, uint8_t,uint8_t, uint8_t,uint8_t);
	void set_alarm_a2(ALARM_A2_TYPE, uint8_t, uint8_t,uint8_t);
	void disable_alarm(ALARM_NUMBER);
};

#endif
