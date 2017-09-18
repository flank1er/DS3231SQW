#ifndef __RTC_DS1307_H__
#define __RTC_DS1307_H__

#include "Arduino.h"
#include "Wire.h"

#define DS1307_I2C_ADDRESS 0x68
#define DS1307_CALENDAR 0x00
#define DS1307_CONTROL_ADDRESS 0x07

enum FREQ { NONE=0x0, FREQ_1HZ=0x10, FREQ_1024HZ=0x08, FREQ_4096HZ=0x11, FREQ_8192HZ=0x12, FREQ_32768HZ=0x13};
enum FIELD { YEAR=6, MONTH=5, DATE=4, DAY=3, HOUR=2, MINUTE=1, SECOND=0};
enum STATUS { DISABLE=0, ENABLE=1};

class DS1307 {
protected:
	FREQ frequency;
	bool sqw;
	uint16_t freq32;
	uint8_t control_address;
	int adjust;
	bool auto_update;
	uint8_t cal[7];
	unsigned long last_update;
	unsigned long last_millis;
	unsigned long last_date;
	unsigned long update_period;
	uint8_t dec_bcd(uint8_t dec) { return (dec/10*16) + (dec%10);};
	uint8_t bcd_dec(uint8_t bcd) { return (bcd-6*(bcd>>4));};
	void set_address(const uint8_t, const uint8_t);
	void calendar_update();
	void request_date();
	void add_day();
	uint8_t get_week_day(uint8_t, uint8_t, uint8_t);
public:
	DS1307 ();
	DS1307 operator++(int);  //posfix
	void set_control(FREQ);
	void set_date(const uint8_t, const uint8_t, const uint8_t, const uint8_t, const uint8_t, const uint8_t, const uint8_t);
	void set_date(long);
	void set_self_update(STATUS);
	void set_adjust(int value) { adjust=value;};
	void set_rtc_adjust(int adjust);
	void stop_clock();
	void force_update();
	uint8_t get(FIELD);
	void print_calendar();
};

#endif
