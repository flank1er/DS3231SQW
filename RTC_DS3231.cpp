#include "RTC_DS3231.h"

#define DS3231_I2C_ADDRESS			0x68
#define DS3231_ALARM1_ADDRESS		0x07
#define DS3231_ALARM2_ADDRESS		0x0B
#define DS3231_CONTROL_ADDRESS 		0x0E
#define DS3231_STATUS_ADDRESS  		0x0F
#define DS3231_AGING_ADDRESS		0x10
#define DS3231_TEMPERATURE_ADDRESS  0x11

DS3231::DS3231() {
    control_address=DS3231_CONTROL_ADDRESS;
    last_millis=millis();
    adjust=0;
    sqw=false;
    set_self_update(DISABLE);
    freq32=32768;
    update_period=0;
	temp_LSB=0;
	temp_MSB=0;
	control=0;
}

char DS3231::get_temperature(){

    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(DS3231_TEMPERATURE_ADDRESS);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_I2C_ADDRESS, 2);
    temp_MSB = Wire.read();
    temp_LSB = Wire.read() >> 6;

	return temp_MSB;
}

char* DS3231::get_temperature_fraction() {
	char* ret="";
	switch (temp_LSB) {
	case 0:
		ret=".00"; break;
	case 1:
		ret=".25"; break;
	case 2:
		ret=".50"; break;
	case 3:
		ret=".75";
	}

	return ret;
}
// CONTROL REGISTER /////////////
void DS3231::preset_control(uint8_t value) {
    set_address(DS3231_CONTROL_ADDRESS, value);
    update_status_control();
}

void DS3231::set_control(uint8_t value) {
    alarm.control |= value;
    set_address(DS3231_CONTROL_ADDRESS, alarm.control);
    update_status_control();
}

void DS3231::reset_control(uint8_t value) {
    alarm.control &= ~value;
    set_address(DS3231_CONTROL_ADDRESS, alarm.control);
    update_status_control();
}

uint8_t DS3231::get_control() {
    return alarm.control;
}

bool DS3231::is_alarm(ALARM_NUMBER num){
	bool ret=false;
	uint8_t value=get_status();
	if (alarm.status & num)
	{
		reset_status(num);
		ret=true;
	}
	return ret;
}

void DS3231::set_sqw(const uint8_t freq){
	switch (freq) {
	case DS3231_1HZ:
		frequency=FREQ_1HZ; break;
	case DS3231_1024HZ:
		frequency=FREQ_1024HZ; break;
	case DS3231_4096HZ:
		frequency=FREQ_4096HZ; break;
	case DS3231_8192HZ:
		frequency=FREQ_8192HZ;
	}

	alarm.control|=freq;
	set_address(DS3231_CONTROL_ADDRESS, alarm.control);
	update_status_control();
}

// AGING OFFSET REGISTER //////////
void DS3231::set_aging(uint8_t value) {
	set_address(DS3231_AGING_ADDRESS, value);
    update_status_control();
}

uint8_t DS3231::get_aging() {
    return alarm.aging;
}

// STATUS REGISTER //////////////////

void DS3231::set_status(uint8_t status) {
	if (alarm.status & DS3231_32KHZ)
		frequency=FREQ_32768HZ;

	alarm.status |= status;
    set_address(DS3231_STATUS_ADDRESS, alarm.status);
	update_status_control();
}

void DS3231::reset_status(uint8_t status) {
    alarm.status &= ~status;
    set_address(DS3231_STATUS_ADDRESS, alarm.status);
    update_status_control();
}

uint8_t DS3231::get_status() {
	return alarm.status;
}

//////////////////////////////////////////////////
void DS3231::update_status_control() {
    Wire.beginTransmission(DS1307_I2C_ADDRESS);
    Wire.write(DS1307_CONTROL_ADDRESS);
    Wire.endTransmission();

	char Buffer[10];
	char * ptr= (char*)Buffer;
    Wire.requestFrom(DS1307_I2C_ADDRESS,10);
	for(char i=0; i<10; i++)
		Buffer[i]=Wire.read();

	alarm=*(ALARM*)ptr;

}

void DS3231::force_update() {
    Wire.beginTransmission(DS1307_I2C_ADDRESS);
    Wire.write(DS1307_CALENDAR);
    Wire.endTransmission();

    Wire.requestFrom(DS1307_I2C_ADDRESS, DS1307_CONTROL_ADDRESS+10);

    for(uint8_t i=0; i<(DS1307_CONTROL_ADDRESS+10); i++)
	{
        if (i==2)
			cal[i]=bcd_dec(Wire.read() & 0x3f);
		else if (i < DS1307_CONTROL_ADDRESS)
			cal[i]=bcd_dec(Wire.read());
		else
			Wire.read();
	}


    last_millis=millis();
    last_update=last_millis;
    last_date=cal[SECOND] + cal[MINUTE]*60;

    uint32_t delta; // 3600*cal[HOUR] don't work:(
    for(int i=0;i<cal[HOUR];i++)
        delta+=3600;

    last_date+=delta;

	update_status_control();
}

void DS3231::set(uint8_t value){
    set_address(DS3231_ALARM1_ADDRESS, value);
    update_status_control();
}

void DS3231::set_alarm_a1(ALARM_A1_TYPE type, uint8_t sec, uint8_t min, uint8_t hour ,uint8_t day) {
    //// W R I T E /////////////////////////////////////
    set_control(DS3231_A1IE|DS3231_INTCH);

    Wire.beginTransmission(DS1307_I2C_ADDRESS);
    Wire.write(DS3231_ALARM1_ADDRESS);
    Wire.write(dec_bcd(sec)|((type<<7) & 0x80));
    Wire.write(dec_bcd(min)|((type<<6) & 0x80));
    Wire.write(dec_bcd(hour)|((type<<5) & 0x80));
    Wire.write(dec_bcd(day)|((type<<4) & 0x80)|((type<<2) & 0x40));
    Wire.endTransmission();
    // read for check ///////////////////////////////////
    update_status_control();
}

void DS3231::set_alarm_a2(ALARM_A2_TYPE type, uint8_t min, uint8_t hour ,uint8_t day) {
    //// W R I T E /////////////////////////////////////
    set_control(DS3231_A2IE|DS3231_INTCH);

    Wire.beginTransmission(DS1307_I2C_ADDRESS);
    Wire.write(DS3231_ALARM2_ADDRESS);
    Wire.write(dec_bcd(min)|((type<<6) & 0x80));
    Wire.write(dec_bcd(hour)|((type<<5) & 0x80));
    Wire.write(dec_bcd(day)|((type<<4) & 0x80)|((type<<2) & 0x40));
    Wire.endTransmission();
    // read for check ///////////////////////////////////
    update_status_control();
}

void DS3231::disable_alarm(ALARM_NUMBER num) {
	if (num == ALARM_A1)
		reset_control(DS3231_A1IE);
	else
		reset_control(DS3231_A2IE);
}

void DS3231::print_bcd(uint8_t bcd){
	static const char symbol[] ="0123456789ABCDEF";
	Serial.print("0x");
	Serial.print((char)symbol[bcd>>4]);
	Serial.print((char)symbol[bcd&0x0f]);
};

void DS3231::print_alarm_1() {
	if (alarm.a1m1)
		Serial.print("A1M1: ON");
	else
		Serial.print("A1M1: OFF");
	Serial.print(" A1 second: ");
//	print_bcd(alarm.second_a1);
	Serial.print(alarm.second_a1,HEX);
	Serial.println("");

    if (alarm.a1m2)
        Serial.print("A1M2: ON");
    else
        Serial.print("A1M2: OFF");
    Serial.print(" A1 minute: ");
	Serial.print(alarm.minute_a1,HEX);
//    print_bcd((uint8_t)alarm.minute_a1);
    Serial.println("");

    if (alarm.a1m3)
        Serial.print("A1M3: ON");
    else
        Serial.print("A1M3: OFF");
    Serial.print(" A1 hour: ");
	Serial.print(alarm.hour_a1,HEX);
//    print_bcd((uint8_t)alarm.hour_a1);
    Serial.println("");

    if (alarm.a1m4)
        Serial.print("A1M4: ON");
    else
        Serial.print("A1M4: OFF");
    if (alarm.dydt_a1)
        Serial.print(" DY/DT: ON, day is: ");
    else
        Serial.print(" DY/DT: OFF, date is: ");

	Serial.print(alarm.day_date_a1,HEX);
//    print_bcd((uint8_t)alarm.day_date_a1);
    Serial.println("");
}

void DS3231::print_alarm_2() {
    if (alarm.a2m2)
        Serial.print("A2M2: ON");
    else
        Serial.print("A2M2: OFF");
    Serial.print(" A2 minute: ");
	Serial.print(alarm.minute_a2,HEX);
//    print_bcd((uint8_t)alarm.minute_a2);
    Serial.println("");

    if (alarm.a2m3)
        Serial.print("A2M3: ON");
    else
        Serial.print("A2M3: OFF");
    Serial.print(" A2 hour: ");
	Serial.print(alarm.hour_a2,HEX);
//    print_bcd((uint8_t)alarm.hour_a2);
    Serial.println("");

    if (alarm.a2m4)
        Serial.print("A2M4: ON");
    else
        Serial.print("A2M4: OFF");
    if (alarm.dydt_a2)
        Serial.print(" DY/DT: ON, day is: ");
    else
        Serial.print(" DY/DT: OFF, date is: ");

	Serial.print(alarm.day_date_a2,HEX);
//    print_bcd((uint8_t)alarm.day_date_a2);
    Serial.println("");
}

void DS3231::print_calendar() {
	DS1307::print_calendar();
	Serial.print("Temperature: "); Serial.print((byte)get_temperature());
	Serial.println(get_temperature_fraction());
	Serial.print("Control: "); Serial.print(get_control(),BIN);
	Serial.print(" Status: "); Serial.println(get_status(),BIN);
}

