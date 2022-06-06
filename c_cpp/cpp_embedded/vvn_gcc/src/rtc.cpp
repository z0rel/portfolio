/*
 * rtc.cpp
 *
 * Created: 09.03.2022 13:00:15
 *  Author: artem
 */

#include "rtc.h"
#include "../arduino/Arduino.h"


void print_rtc_value()
{
    rtc_set_read_mode_start();
    uint8_t century       = RTC_GET_BYTE_CENTURY;
    century               = RTC_NORMALIZE_BITS(century);
    uint8_t seconds       = RTC_GET_BYTE_SECONDS;
    seconds               = RTC_NORMALIZE_BITS(seconds);
    uint8_t minutes       = RTC_GET_BYTE_MINUTES;
    minutes               = RTC_NORMALIZE_BITS(minutes);
    uint8_t hours         = RTC_GET_BYTE_HOUR;
    hours                 = RTC_NORMALIZE_BITS(hours);
    uint8_t day_of_months = RTC_GET_BYTE_DAY_OF_MONTH;
    day_of_months         = RTC_NORMALIZE_BITS(day_of_months);
    uint8_t day_of_weeks  = RTC_GET_BYTE_DAY_OF_WEEK;
    day_of_weeks          = RTC_NORMALIZE_BITS(day_of_weeks);
    uint8_t month         = RTC_GET_BYTE_MONTH;
    month                 = RTC_NORMALIZE_BITS(month);
    uint8_t year          = RTC_GET_BYTE_YEAR;
    year                  = RTC_NORMALIZE_BITS(year);
    rtc_set_read_write_mode_end();

    Serial.print(RTC_GET_HOUR10(hours));
    Serial.print(RTC_GET_HOUR(hours));
    Serial.print(":");
    Serial.print(RTC_GET_MINUTES10(minutes));
    Serial.print(RTC_GET_MINUTES(minutes));
    Serial.print(":");
    Serial.print(RTC_GET_SECONDS10(seconds));
    Serial.print(RTC_GET_SECONDS(seconds));
    Serial.print(" ");
    Serial.print(RTC_GET_CENTURY10(century));
    Serial.print(RTC_GET_CENTURY(century));
    Serial.print(RTC_GET_YEAR10(year));
    Serial.print(RTC_GET_YEAR(year));
    Serial.print(".");
    Serial.print(RTC_GET_MONTH10(month));
    Serial.print(RTC_GET_MONTH(month));
    Serial.print(".");
    Serial.print(RTC_GET_DAY_OF_MONTH10(day_of_months));
    Serial.print(RTC_GET_DAY_OF_MONTH(day_of_months));
    Serial.println();
}


void DateTime::load()
{
    rtc_set_read_mode_start();

    this->tm_sec     = RTC_GET_BYTE_SECONDS;
    this->tm_min     = RTC_GET_BYTE_MINUTES;
    this->tm_hour    = RTC_GET_BYTE_HOUR;
    this->tm_mday    = RTC_GET_BYTE_DAY_OF_MONTH;
    this->tm_mon     = RTC_GET_BYTE_MONTH;
    this->tm_year    = RTC_GET_BYTE_YEAR;
    this->tm_century = RTC_GET_BYTE_CENTURY;
    this->tm_wday    = RTC_GET_BYTE_DAY_OF_WEEK;

    rtc_set_read_write_mode_end();

    this->tm_sec = RTC_NORMALIZE_BITS(this->tm_sec);
    this->tm_sec = RTC_GET_MINUTES10(this->tm_sec) * 10 + RTC_GET_MINUTES(this->tm_sec);

    this->tm_min = RTC_NORMALIZE_BITS(this->tm_hour);
    this->tm_min = RTC_GET_MINUTES10(this->tm_min) * 10 + RTC_GET_MINUTES(this->tm_min);

    this->tm_hour = RTC_NORMALIZE_BITS(this->tm_hour);
    this->tm_hour = RTC_GET_HOUR10(this->tm_hour) * 10 + RTC_GET_HOUR(this->tm_hour);

    this->tm_mday = RTC_NORMALIZE_BITS(this->tm_mday);
    this->tm_mday = RTC_GET_DAY_OF_MONTH10(this->tm_mday) * 10 + RTC_GET_DAY_OF_MONTH(this->tm_mday);

    this->tm_mon = RTC_NORMALIZE_BITS(this->tm_mon);
    this->tm_mon = RTC_GET_MONTH10(this->tm_mon) * 10 + RTC_GET_MONTH(this->tm_mon);

    this->tm_year = RTC_NORMALIZE_BITS(this->tm_year);
    this->tm_year = RTC_GET_YEAR10(this->tm_year) * 10 + RTC_GET_YEAR(this->tm_year);

    this->tm_century = RTC_NORMALIZE_BITS(this->tm_century);
    this->tm_century = RTC_GET_CENTURY10(this->tm_century) * 10 + RTC_GET_CENTURY(this->tm_century);

    this->tm_wday = RTC_NORMALIZE_BITS(this->tm_wday);
    this->tm_wday = RTC_GET_DAY_OF_MONTH10(this->tm_wday) * 10 + RTC_GET_DAY_OF_MONTH(this->tm_wday);
}
