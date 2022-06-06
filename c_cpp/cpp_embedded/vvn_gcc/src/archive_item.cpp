/*
 * archive_item.cpp
 *
 * Created: 14.03.2022 15:45:54
 *  Author: artem
 */


#include <avr/eeprom.h>

#include "archive_item.h"

#define MAX_ARCHIVE_ITEMS 494

uint8_t EEMEM eeprom_archive_array[MAX_ARCHIVE_ITEMS * sizeof(Archive::BitArchive)]; //  = UART_Baudrate.B_9600;

// TODO: для экономии места - сделать 2 типа записи.
// 1 - запись начала отсчета времени с типом интервала логгирования
// Такую запись нужно устанавливать при 
// - смене типа интервала логгирования, 
// - при включении устройства с активным логгированием 
// - при активации логгирования у включенного устройства
// 2 - запсись данных

// Запись в лог делать по событию часов а не через задержку

// TODO: Если датчик не подключен, в журнал не писать
// TODO: Если датчик отключен - записать в журнал факт отключения датчика. Использовать незадействованные комбинации в битах времени. Аналогично - о подключении
// TODO: Разместить дубликат журнала в SRAM для ускорения чтения по Modbus

void Archive::ArchiveItem::pack(const DateTime &dtime, uint16_t kinematic_viscosity_adc, uint16_t temperature_adc, uint16_t dencity_adc)
{
    this->packed_value.month                   = dtime.tm_mon;
    this->packed_value.day_of_month            = dtime.tm_mday;
    this->packed_value.dencity_adc             = dencity_adc;
    this->packed_value.hours                   = dtime.tm_hour;
    this->packed_value.kinematic_viscosity_adc = kinematic_viscosity_adc;
    this->packed_value.minutes                 = dtime.tm_min;
    this->packed_value.month                   = dtime.tm_mon;
    this->packed_value.seconds                 = dtime.tm_sec;
    this->packed_value.setted                  = 1;
    this->packed_value.temperature_adc         = temperature_adc;
    this->packed_value.year                    = dtime.tm_year;
}


void Archive::ArchiveItem::unpack(DateTime &dtime, uint16_t &kinematic_viscosity_adc, uint16_t &temperature_adc, uint16_t &dencity_adc)
{
    dtime.tm_mon            = this->packed_value.month;
    dtime.tm_mday           = this->packed_value.day_of_month;
    dencity_adc             = this->packed_value.dencity_adc;
    dtime.tm_hour           = this->packed_value.hours;
    kinematic_viscosity_adc = this->packed_value.kinematic_viscosity_adc;
    dtime.tm_min            = this->packed_value.minutes;
    dtime.tm_mon            = this->packed_value.month;
    dtime.tm_sec            = this->packed_value.seconds;
    temperature_adc         = this->packed_value.temperature_adc;
    dtime.tm_year           = this->packed_value.year;
}


void Archive::ArchiveItem::load(uint16_t idx)
{
    if (idx < MAX_ARCHIVE_ITEMS) {
        eeprom_read_block(static_cast<uint8_t *>(static_cast<uint8_t *>(eeprom_archive_array) + idx * sizeof(Archive::BitArchive)), reinterpret_cast<uint8_t *>(&this->packed_value),
                          sizeof(Archive::BitArchive));
    }
}


void Archive::ArchiveItem::save(uint16_t idx)
{
    if (idx < MAX_ARCHIVE_ITEMS) {
        eeprom_update_block(reinterpret_cast<uint8_t *>(&this->packed_value), static_cast<uint8_t *>(static_cast<uint8_t *>(eeprom_archive_array) + idx * sizeof(Archive::BitArchive)),
                            sizeof(Archive::BitArchive));
    }
}


void Archive::ArchiveItem::init_eeprom_array()
{
    for (uint16_t i = 0; i < MAX_ARCHIVE_ITEMS * sizeof(Archive::BitArchive); ++i) {
        eeprom_update_byte(eeprom_archive_array + i, 0);
    }
}


bool Archive::operator<(const Archive::BitArchive &lhs, const Archive::BitArchive &rhs)
{
    if (lhs.year < rhs.year) {
        return true;
    }
    else if (lhs.year == rhs.year) {
        if (lhs.month < rhs.month) {
            return true;
        }
        else if (lhs.month == rhs.month) {
            if (lhs.day_of_month < rhs.day_of_month) {
                return true;
            }
            else if (lhs.day_of_month == rhs.day_of_month) {
                if (lhs.hours < rhs.hours) {
                    return true;
                }
                else if (lhs.hours == rhs.hours) {
                    if (lhs.minutes < rhs.minutes) {
                        return true;
                    }
                    else if (lhs.minutes == rhs.minutes) {
                        if (lhs.seconds < rhs.seconds) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}


void Archive::ArchiveItem::find_start_and_count(uint16_t &start_idx, uint16_t &count_idx)
{
    Archive::BitArchive min_packed_value;
    Archive::BitArchive packed_value;

    eeprom_read_block(static_cast<uint8_t *>(static_cast<uint8_t *>(eeprom_archive_array) + 0 * sizeof(Archive::BitArchive)), reinterpret_cast<uint8_t *>(&min_packed_value),
                      sizeof(Archive::BitArchive));

    if (!min_packed_value.setted) {
        start_idx = 0;
        count_idx = 0;
        return;
    }

    uint16_t min_idx       = 1;
    uint16_t max_count_idx = 1;

    for (uint16_t i = 1; i < MAX_ARCHIVE_ITEMS * sizeof(Archive::BitArchive); ++i) {
        eeprom_read_block(static_cast<uint8_t *>(static_cast<uint8_t *>(eeprom_archive_array) + i * sizeof(Archive::BitArchive)), reinterpret_cast<uint8_t *>(&packed_value),
                          sizeof(Archive::BitArchive));
        if (!packed_value.setted) {
		    start_idx = min_idx;
			count_idx = max_count_idx;
            break;
        }
        else {
            ++max_count_idx;
            if (packed_value < min_packed_value) {
                min_packed_value = packed_value;
                min_idx          = i;
            }
        }
    }
}
