/*
 * rtc.h
 *
 * Created: 09.03.2022 13:00:48
 *  Author: artem
 */


#ifndef RTC_H_
#define RTC_H_

#include <avr/sfr_defs.h>
#include <stdint.h>


#define RTC_ADDR_CENTURY 0x7FF8
#define RTC_ADDR_SECONDS 0x7FF9
#define RTC_ADDR_MINUTES 0x7FFA
#define RTC_ADDR_HOURS 0x7FFB
#define RTC_ADDR_DAYS_OF_WEEK 0x7FFC
#define RTC_ADDR_DAYS_OF_MONTH 0x7FFD
#define RTC_ADDR_MONTH 0x7FFE
#define RTC_ADDR_YEAR 0x7FFF
#define RTC_BIT_OFFSET >> 4


// 0 -> 0
// 1 -> 1
// 2 -> 2
// 5 -> 5

// 4 -> 6
// 6 -> 4

// 3 -> 7
// 7 -> 3

#define RTC_NORMALIZE_BITS(b) (((b & _BV(3)) << (7 - 3)) | ((b & _BV(7)) >> (7 - 3)) | ((b & _BV(4)) << (6 - 4)) | ((b & _BV(6)) >> (6 - 4)) | (b & (_BV(0) | _BV(1) | _BV(2) | _BV(5))))

#define RTC_GET_BYTE_YEAR (*reinterpret_cast<uint8_t *>(RTC_ADDR_YEAR))
#define RTC_GET_YEAR10(b) static_cast<unsigned int>(((b) & (_BV(7) | _BV(6) | _BV(5) | _BV(4))) RTC_BIT_OFFSET)
#define RTC_GET_YEAR(b) static_cast<unsigned int>((b) & (_BV(3) | _BV(2) | _BV(1) | _BV(0)))

#define RTC_GET_BYTE_MONTH (*reinterpret_cast<uint8_t *>(RTC_ADDR_MONTH))
#define RTC_GET_MONTH10(b) static_cast<unsigned int>(((b)&_BV(4))RTC_BIT_OFFSET)
#define RTC_GET_MONTH(b) static_cast<unsigned int>((b) & (_BV(3) | _BV(2) | _BV(1) | _BV(0)))

#define RTC_GET_BYTE_DAY_OF_WEEK (*reinterpret_cast<uint8_t *>(RTC_ADDR_DAYS_OF_WEEK))
#define RTC_GET_DAY_OF_WEEK(b) static_cast<unsigned int>((b) & (_BV(2) | _BV(1) | _BV(0)))

#define RTC_GET_BYTE_DAY_OF_MONTH (*reinterpret_cast<uint8_t *>(RTC_ADDR_DAYS_OF_MONTH))
#define RTC_GET_DAY_OF_MONTH10(b) static_cast<unsigned int>(((b) & (_BV(5) | _BV(4))) RTC_BIT_OFFSET)
#define RTC_GET_DAY_OF_MONTH(b) static_cast<unsigned int>((b) & (_BV(3) | _BV(2) | _BV(1) | _BV(0)))

#define RTC_GET_BYTE_HOUR (*reinterpret_cast<uint8_t *>(RTC_ADDR_HOURS))
#define RTC_GET_HOUR10(b) static_cast<unsigned int>(((b) & (_BV(5) | _BV(4))) RTC_BIT_OFFSET)
#define RTC_GET_HOUR(b) static_cast<unsigned int>((b) & (_BV(3) | _BV(2) | _BV(1) | _BV(0)))

#define RTC_GET_BYTE_MINUTES (*reinterpret_cast<uint8_t *>(RTC_ADDR_MINUTES))
#define RTC_GET_MINUTES10(b) static_cast<unsigned int>(((b) & (_BV(6) | _BV(5) | _BV(4))) RTC_BIT_OFFSET)
#define RTC_GET_MINUTES(b) static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(RTC_ADDR_MINUTES) & (_BV(3) | _BV(2) | _BV(1) | _BV(0)))

#define RTC_GET_BYTE_SECONDS (*reinterpret_cast<uint8_t *>(RTC_ADDR_SECONDS))
#define RTC_GET_SECONDS10(b) static_cast<unsigned int>(((b) & (_BV(6) | _BV(5) | _BV(4))) RTC_BIT_OFFSET)
#define RTC_GET_SECONDS(b) static_cast<unsigned int>((b) & (_BV(3) | _BV(2) | _BV(1) | _BV(0)))

#define RTC_GET_BYTE_CENTURY (*reinterpret_cast<uint8_t *>(RTC_ADDR_CENTURY))
#define RTC_GET_CENTURY10(b) static_cast<unsigned int>(((b) & (_BV(5) | _BV(4))) RTC_BIT_OFFSET)
#define RTC_GET_CENTURY(b) static_cast<unsigned int>((b) & (_BV(3) | _BV(2) | _BV(1) | _BV(0)))


#define RTC_WRITE_BIT 7
#define RTC_WRITE_BIT_MAPPED 3
#define RTC_READ_BIT 6
#define RTC_READ_BIT_MAPPED 3
#define RTC_ADDR_RW 0x7FF8

#define RTC_ADDR_OSC 0x7FF9
#define RTC_OSC_BIT 7
#define RTC_OSC_BIT_MAPPED 3

#define RTC_BF_BIT 7
#define RTC_BF_BIT_MAPPED 3
#define RTC_FT_BIT 6
#define RTC_FT_BIT_MAPPED 4
#define RTC_ADDR_BF_FT 0x7FFC


inline bool rtc_battary_low()
{
    uint8_t *p = reinterpret_cast<uint8_t *>(RTC_ADDR_BF_FT);
    return (*p & _BV(RTC_BF_BIT_MAPPED)) == 0;
}


inline bool rtc_battary_ok()
{
    uint8_t *p = reinterpret_cast<uint8_t *>(RTC_ADDR_BF_FT);
    return (*p & _BV(RTC_BF_BIT_MAPPED)) != 0;
}


inline void rtc_set_read_mode_start()
{
    uint8_t *p = reinterpret_cast<uint8_t *>(RTC_ADDR_RW);
    *p         = (*p & ~_BV(RTC_WRITE_BIT_MAPPED)) | _BV(RTC_READ_BIT_MAPPED);
}

inline void rtc_set_read_write_mode_end()
{
    uint8_t *p = reinterpret_cast<uint8_t *>(RTC_ADDR_RW);
    *p         = (*p & (~_BV(RTC_WRITE_BIT_MAPPED) & ~_BV(RTC_READ_BIT_MAPPED)));
}


inline void rtc_set_write_mode_start()
{
    uint8_t *p = reinterpret_cast<uint8_t *>(RTC_ADDR_RW);
    *p         = (*p & ~_BV(RTC_READ_BIT_MAPPED)) | _BV(RTC_WRITE_BIT_MAPPED);
}


inline void rtc_init()
{
    rtc_set_read_write_mode_end();
    uint8_t *p = reinterpret_cast<uint8_t *>(RTC_ADDR_OSC);
    *p &= ~_BV(RTC_OSC_BIT_MAPPED);
    p = reinterpret_cast<uint8_t *>(RTC_ADDR_BF_FT);
    *p &= ~_BV(RTC_FT_BIT_MAPPED);
}


class DateTime {
  public:
    uint8_t tm_sec; //  seconds after the minute	0-61*
    uint8_t tm_min; //  minutes after the hour	0-59
    uint8_t tm_hour; // hours since midnight	0-23
    uint8_t tm_mday; // day of the month	1-31
    uint8_t tm_mon; //  months since January	0-11
    uint8_t tm_year; // years since 00-99
    uint8_t tm_century; //
    uint8_t tm_wday; // days since Sunday	0-6

    void load();
};


#endif /* RTC_H_ */
