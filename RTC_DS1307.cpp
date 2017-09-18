#include <RTC_DS1307.h>

DS1307::DS1307() {
	control_address=DS1307_CONTROL_ADDRESS;
	last_millis=millis();
	adjust=0;
	sqw=false;
	set_self_update(DISABLE);
	freq32=32768;
	update_period=0;
	auto_update=true;
}

DS1307 DS1307::operator++(int) {
	sqw=true;
	bool second=false;
	// for tackting from 32kHz
/*	if((frequency == FREQ_32768HZ) && (freq32 > 0)) {
		--freq32;
		if (!freq32) {
			second=true;
			freq32=32768;
		};
	}
*/
	if (frequency == FREQ_1HZ || second)
		++last_date;

	if (last_date >= 86400) //midnight
		add_day();
}

void DS1307::stop_clock() {
     Wire.beginTransmission(DS1307_I2C_ADDRESS);
     Wire.write(DS1307_CALENDAR);

     Wire.write(0x80);
     Wire.endTransmission();
}

void DS1307::force_update() {
	Wire.beginTransmission(DS1307_I2C_ADDRESS);
	Wire.write(DS1307_CALENDAR);
	Wire.endTransmission();

	Wire.requestFrom(DS1307_I2C_ADDRESS, 7);
	for(uint8_t i=0; i<DS1307_CONTROL_ADDRESS; i++) {
		if (i==0)
			cal[i]=bcd_dec(Wire.read() & 0x7f);
		else if (i==2)
			cal[i]=bcd_dec(Wire.read() & 0x3f);
		else
			cal[i]=bcd_dec(Wire.read());
	}

	last_millis=millis();
	last_update=last_millis;
	last_date=cal[SECOND] + cal[MINUTE]*60;

	uint32_t delta; // 3600*cal[HOUR] don't work:(
	for(int i=0;i<cal[HOUR];i++)
		delta+=3600;

	last_date+=delta;
}

void DS1307::set_address(const uint8_t address, const uint8_t value)
{
	Wire.beginTransmission(DS1307_I2C_ADDRESS);
	Wire.write(address);
	Wire.write(value);
	Wire.endTransmission();
}

void DS1307::set_control(FREQ value)
{
	frequency=value;
	set_address(control_address, value);
}

void DS1307::set_rtc_adjust(int adjust) {
	force_update();

    last_date+=adjust;
    cal[SECOND]=last_date%60;
    cal[MINUTE]=(last_date%3600)/60;
    cal[HOUR]=(uint8_t)(last_date/3600);

	Wire.beginTransmission(DS1307_I2C_ADDRESS);
	Wire.write(DS1307_CALENDAR);
	for(uint8_t i=0;i<DS1307_CONTROL_ADDRESS; i++)
		Wire.write(dec_bcd(cal[i]));
	Wire.endTransmission();
}


void DS1307::add_day() {
	last_date=0;
	cal[DAY]=cal[DAY]%7+1;

    uint8_t days;
    switch(cal[MONTH]) {
    case 4:
    case 6:
    case 9:
    case 11:
        days=30; break;
    case 2:
        days=(cal[YEAR] % 4 == 0) ? 29 :28; break;
    default: days=31;
    }

	if (cal[DATE] == days) {
		cal[DATE]=1;
		cal[MONTH]=cal[MONTH]+1;
	} else
		cal[DATE]=cal[DATE]+1;

	// new year
	if (cal[MONTH] > 12) {
		cal[MONTH] = 1;
		cal[YEAR] = cal[YEAR] + 1;
	}
}

void DS1307::calendar_update(){
	unsigned long current=millis();
	if (!sqw) {
		unsigned long sec=last_date+(current-last_millis)/1000;
		cal[SECOND]=sec%60;
		cal[MINUTE]=(sec%3600)/60;
		cal[HOUR]=(uint8_t)(sec/3600);

		// midnight
		if (sec >= (86400+adjust)) {
			add_day();
			last_millis=current;
		}

	} else {
	    cal[SECOND]=last_date%60;
    	cal[MINUTE]=(last_date%3600)/60;
	    cal[HOUR]=(uint8_t)(last_date/3600);
	}

	last_update=current;
}

uint8_t DS1307::get(FIELD value) {
	if ((millis()-last_update) > 1000)
		request_date();

	return cal[value];
}

void DS1307::request_date() {
	if( !(auto_update || sqw) || ((update_period > 0) && ((millis()-last_millis) >= update_period)))
		force_update();
	else
		calendar_update();
}

void DS1307::set_self_update(STATUS value) {
	auto_update = (value) ? true: false;
	sqw=false;
};

void DS1307::set_date(const uint8_t year, const uint8_t month, const uint8_t date, const uint8_t day,
                        const uint8_t hour, const uint8_t minute, const uint8_t second) {
	last_millis=millis();
	Wire.beginTransmission(DS1307_I2C_ADDRESS);
	Wire.write(DS1307_CALENDAR);

	Wire.write(second);
	Wire.write(minute);
	Wire.write(hour);
	Wire.write(day);
	Wire.write(date);
	Wire.write(month);
	Wire.write(year);
	Wire.endTransmission();
}


// from https://github.com/PaulStoffregen/Time
void DS1307::set_date(long value) {

	uint32_t time = (uint32_t)value;
	cal[SECOND] = time % 60;
	time /= 60; // now it is minutes
	cal[MINUTE] = time % 60;
	time /= 60; // now it is hours
	cal[HOUR] = time % 24;
	time /= 24; // now it is days
	cal[DAY] = ((time + 3) % 7) + 1;  // Monday is day 1

	uint8_t year = 0;
	unsigned long days = 0;
	while((unsigned)(days += (!(year %4) ? 366 : 365)) <= time) {
		year++;
	}
	cal[YEAR] = year-30; // year is offset from 2000

	days -= !(year %4) ? 366 : 365;
	time  -= days; // now it is days in this year, starting at 0

	static uint8_t day_count[12]={31,28,31,30,31,30,31,31,30,31,30,31};

	days=0;
	uint8_t month=0;
	uint8_t monthLength=0;
	for (month=0; month<12; month++)
	{
		if (month==1) { // february
			if (!(year % 4))
				monthLength=29;
			else
				monthLength=28;
		} else
			monthLength = day_count[month];

		if (time >= monthLength)
			time -= monthLength;
		else
			break;
	}
	cal[MONTH] = month + 1;  // jan is month 1
	cal[DATE] = time + 1;     // day of month


	last_millis=millis();
	Wire.beginTransmission(DS1307_I2C_ADDRESS);
	Wire.write(DS1307_CALENDAR);

	for(char i=0;i<7;i++)
		Wire.write(dec_bcd(cal[i]));

	Wire.endTransmission();
}


// algorithm - https://litemind.com/how-to-become-a-human-calendar/
// algorithm - https://lifehacker.ru/2016/02/19/kakoj-den-nedeli/
uint8_t DS1307::get_week_day(uint8_t y, uint8_t m, uint8_t date){
	uint8_t mm[12]={1,4,4,0,2,5,0,3,6,1,4,6};
    uint8_t yy=(6+ y +(y>>2))%7;

    if (((y%4) == 0) && m <=2) // leap year
        yy=(date+yy+mm[m]-1)%7;
    else
        yy=(date+yy+mm[m])%7;

    return (yy >= 2) ? --yy: yy+6;
}

void DS1307::print_calendar() {
    Serial.print("year: "); Serial.print(get(YEAR));
    Serial.print(" month: "); Serial.print(get(MONTH));
    Serial.print(" date: "); Serial.print(get(DATE));
    Serial.print(" day: "); Serial.print(get(DAY));
    Serial.print(" hour: "); Serial.print(get(HOUR));
    Serial.print(" minute: "); Serial.print(get(MINUTE));
    Serial.print(" second: "); Serial.println(get(SECOND));
}
