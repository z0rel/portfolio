/*
 * archive_item.h
 *
 * Created: 14.03.2022 15:45:19
 *  Author: artem
 */


#ifndef ARCHIVE_ITEM_H_
#define ARCHIVE_ITEM_H_

#include "rtc.h"


// TODO: define PackDatetime
// TODO: define UnpackDatetime
// час:минута:секунда число.месяц.год

namespace Archive {

struct BitArchive {
    uint8_t seconds : 6; // [5 bit - second 0-61]
    uint8_t minutes : 6; // [6 bit - minutes 0-59]
    uint8_t hours : 5; // [5 bit - hours 0-23]
    uint8_t day_of_month : 5; // [5 bit - day of month 0-31]
    uint8_t month : 5; // [4 bit - month 0-11]
    uint8_t year : 5; // [7 bit - year 0-99]

    uint16_t kinematic_viscosity_adc : 10; // Кинематическая вязкость
    uint16_t temperature_adc : 10; //         Значение температуры
    uint16_t dencity_adc : 10; //             Значение плотности
    uint16_t setted : 1; //                   Бит "установлен"

} __attribute__((packed));


bool operator<(const BitArchive &lhs, const BitArchive &rhs);


class ArchiveItem {
  public:
    BitArchive packed_value;

    void pack(const DateTime &dtime, uint16_t kinematic_viscosity_adc, uint16_t temperature_adc, uint16_t dencity_adc);
    void unpack(DateTime &dtime, uint16_t &kinematic_viscosity_adc, uint16_t &temperature_adc, uint16_t &dencity_adc);

    void load(uint16_t idx);
    void save(uint16_t idx);
    static void init_eeprom_array();
    static void find_start_and_count(uint16_t &start_idx, uint16_t &count_idx);
};

} // namespace Archive

// TODO: реализовать функциональность ArchiveLog


#endif /* ARCHIVE_ITEM_H_ */
