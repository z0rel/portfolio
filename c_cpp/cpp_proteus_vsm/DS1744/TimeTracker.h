#pragma once

#include <ctime>
#include <functional>
#include <chrono>
#include <future>
#include <cstdio>
#include <atomic>
#include <time.h>



class TimeTracker
{

	// Хотя структура регистров с двойной буферизацией снижает вероятность
	// чтения неверных данных,  внутренние обновления регистров часов DS1744
	// должны быть остановлены до того, как будут считаны данные часов, чтобы
	// предотвратить чтение данных в процессе передачи. 
	// 
	// Однако остановка процесса обновления внутреннего регистра часов не
	// влияет на точность часов. 
	// 
	// Обновление останавливается, когда в бит чтения R = READ_BIT, бит 6 регистра века записывается 1 (таблица 2). 
	// Пока в этой позиции остается 1, обновление останавливается. 
	// После выдачи останова регистры отражают количество, т. е. день, дату и
	// время, которые были текущими на момент выдачи команды останова. 
	// Однако внутренние регистры часов системы с двойной буферизацией
	// продолжают обновляться, поэтому доступ к данным не влияет на точность
	// часов. Все регистры DS1744 обновляются одновременно после повторного
	// включения внутреннего процесса обновления регистра часов. 
	// 
	// Обновление происходит в течение секунды после того, как бит чтения
	// записывается в 0. 
	// 
	// Бит READ должен быть равен 0 в течение как минимум 500 мкс, чтобы
	// обеспечить обновление внешних регистров.

	// Снова обращаясь к таблице 2, бит 6 регистра управления является битом R (чтение, READ_BIT). 
	// 
	// Установка бита R в 1 останавливает обновление регистров устройства. 
	// 
	// Впоследствии пользователь может считывать значения даты и времени из
	// восьми регистров без возможного изменения их содержимого во время этих
	// операций ввода-вывода. 
	// 
	// Последующий цикл записи 00h в регистр управления для очистки бита R
	// позволяет возобновить операции хронометража с предыдущей уставки.
	// 
	// READ_BIT = 0 => нельзя читать
	// READ_BIT = 1 => можно читать, 
	//     после чтения: 
	//     1. нужно сделать READ_BIT := 0 
	//     2. подождать до следующего чтения минимум 500 мкс

	std::atomic<bool> READ_BIT = false;

	// Как показано в таблице 2, бит 7 регистра управления является битом W
	// (запись, WRITE_BIT). 
	// 
	// Установка бита W в 1 останавливает обновление регистров устройства. 
	// 
	// Впоследствии пользователь может загрузить правильные значения даты и
	// времени во все восемь регистров, 
	// 
	// после чего следует цикл записи 00h в управляющий регистр, чтобы сбросить
	// бит W и передать эти новые настройки в часы, что позволяет возобновить
	// операции хронометража с новой уставки.
	// 
	// WRITE_BIT = 0 => нельзя писать
	// WRITE_BIT = 1 => можно писать, 
	//    после записи:
	//    нужно записать в WRITE_BIT := 0

	std::atomic<bool> WRITE_BIT = false;

	// Предварительно существовавшее содержимое битов регистра управления 0:5
	// (значение века) игнорируется/не модифицируется циклом записи в
	// управление, если биты W или R устанавливаются в 1 в этой операции
	// записи. 
	// 
	// Предварительно существовавшее содержимое битов регистра управления 0:5
	// (значение века) будет изменено циклом записи на управление, если бит W
	// очищается до 0 в этой операции записи.
	// 
	// Предварительно существовавшее содержимое битов регистра управления 0:5
	// (значение столетия) не будет изменено циклом записи в управляющее
	// значение, если бит R очищается до 0 в этой операции записи.

	// TODO: генерировать исключения при чтении без установленного READ_BIT и при записи без установленного WRITE_BIT

	// Тактовый осциллятор можно остановить в любой момент. Для увеличения
	// срока годности осциллятор можно отключить, чтобы свести к минимуму
	// потребление тока от батареи. Бит OSC является старшим разрядом (бит 7)
	// секундных регистров (таблица 2). Установка его на 1 останавливает
	// осциллятор.
	//
	//  __OSC = 0 - клок часов  работает
	//  __OSC = 1 - клок часов не работает

	std::atomic<bool> __OSC = false;


	// Как показано в таблице 2, бит 6 байта дня является битом проверки
	// частоты. 
	// 
	// Когда бит проверки частоты установлен на логическую 1 и генератор
	// работает, младший бит секундного регистра переключается на 512
	// Гц. 
	// 
	// Когда считывается регистр секунд, линия DQ0 переключается на частоту
	// 512 Гц до тех пор, пока условия доступа остаются в силе (т. е. низкий
	// уровень CE, низкий уровень OE, высокий уровень WE и адрес секундного
	// регистра остаются действительными и стабильными).
	bool FREQUENCY_TEST = false; // TODO: это не сделано

	// DS1744 постоянно контролирует напряжение внутренней батареи. Бит флага
	// батареи (бит 7) регистра дня используется для указания диапазона уровня
	// напряжения батареи. 
	// 
	// Этот бит недоступен для записи и всегда должен быть равен 1 при чтении. 
	// 
	// Если всегда присутствует 0, это означает, что источник энергии лития исчерпан, а содержимое RTC и RAM вызывает сомнения.
	// BATTARY_FLAG = 1 - всё хорошо
	// BATTARY_FLAG = 0 - батарейка садится, содержимое часов и памяти может быть неправильным.
	bool BATTARY_FLAG = true;

public:
	std::atomic<time_t> now_counter = time(0); // current date/time based on current system
	std::atomic<time_t> now = time(0); // current date/time based on current system
	std::thread count_thread;
	std::atomic<bool> running = true;

	inline TimeTracker() {
		count_thread = std::thread([this]() {
			while (this->running) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				if (!this->__OSC && !this->WRITE_BIT) {
					// если осциллятор часов включен и запись не выполняется - обновить часы
					++this->now_counter;
					if (!this->READ_BIT) {
						// Если не выполняется чтение - обновить выходной регистр чтения
						this->now = static_cast<time_t>(this->now_counter);
					}
				}
			}
			}
		);
	}
	inline ~TimeTracker() {
		running = false;
		count_thread.join();
	}

	inline void set_read_bit(bool val) { this->READ_BIT = val; }
	inline void set_write_bit(bool val) { this->WRITE_BIT = val; }
	inline void set_osc_bit(bool val) { this->__OSC = val; }
	inline void set_frequency_test_bit(bool val) { this->FREQUENCY_TEST = val; }

	inline bool read_bit() const { return this->READ_BIT; }
	inline bool write_bit() const { return this->WRITE_BIT; }
	inline bool osc_bit() const { return this->__OSC; }
	inline bool frequency_test_bit() const { return this->FREQUENCY_TEST; }
	inline bool battary_flag_bit() const { return this->BATTARY_FLAG; }

private:
	inline unsigned int get_time_component(int tm::* attr) const {
		time_t local_now = now;
		struct tm t;
		localtime_s(&t, &local_now);
		return t.*attr;
	}

	inline void update_date_component_if_possible() {
		if (this->WRITE_BIT) {
			this->now_counter = static_cast<time_t>(this->now);
		}
	}

public:
	static inline uint8_t to_bcd(unsigned int value) {
		unsigned int value_10 = (value / 10) % 10;
		unsigned int value_low = value % 10;
		return (static_cast<uint8_t>(value_10) << 4) | static_cast<uint8_t>(value_low);
	}

	static inline uint8_t from_bcd(uint8_t value) {
		uint8_t low = (value & 0xF);
		uint8_t high = ((value & 0xF0) >> 4) * 10;
		return high + low;
	}


	inline unsigned int second() const { return get_time_component(&tm::tm_sec); }
	inline unsigned int minute() const { return get_time_component(&tm::tm_min); }
	inline unsigned int hour() const { return get_time_component(&tm::tm_hour); }
	inline unsigned int day() const { return get_time_component(&tm::tm_mday); }
	inline unsigned int day_week() const { return get_time_component(&tm::tm_wday); }
	inline unsigned int month() const { return get_time_component(&tm::tm_mon) + 1; }
	inline unsigned int year() const { return get_time_component(&tm::tm_year); }

	inline bool set_second(uint8_t seconds) {
		if (seconds > 60) {
			return false;
		}
		time_t local_now = now;
		struct tm t;
		localtime_s(&t, &local_now);
		t.tm_sec = static_cast<int>(seconds);
		now = mktime(&t);
		update_date_component_if_possible();
		return true;
	}
	inline bool set_minute(uint8_t minutes) {
		if (minutes > 60) {
			return false;
		}
		time_t local_now = now;
		struct tm t;
		localtime_s(&t, &local_now);
		t.tm_min = static_cast<int>(minutes);
		now = mktime(&t);
		update_date_component_if_possible();
		return true;
	}
	inline bool set_hour(uint8_t hours) {
		if (hours > 23) {
			return false;
		}
		time_t local_now = now;
		struct tm t;
		localtime_s(&t, &local_now);
		t.tm_hour = static_cast<int>(hours);
		now = mktime(&t);
		update_date_component_if_possible();
		return true;
	}
	inline bool set_day(uint8_t day) {
		time_t local_now = now;
		struct tm t;
		localtime_s(&t, &local_now);
		switch (t.tm_mon) {
		case 0: // yan
			if (day > 31) {
				return false;
			}
			break;
		case 1: { // feb
			if (t.tm_year > 1899) {
				if ((t.tm_year % 4) == 0) {
					if (day > 29) {
						return false;
					}
				}
				else if (day > 28) {
					return false;
				}
			}
			else {
				if (day > 28) {
					return false;
				}
			}
			break;
		}
		case 2: // mar
			if (day > 31) {
				return false;
			}
			break;
		case 3: // apr
			if (day > 30) {
				return false;
			}
			break;
		case 4: // may
			if (day > 31) {
				return false;
			}
			break;
		case 5: // jun
			if (day > 30) {
				return false;
			}
			break;
		case 6: // jul
			if (day > 31) {
				return false;
			}
			break;
		case 7: // aug
			if (day > 31) {
				return false;
			}
			break;
		case 8: // sep
			if (day > 30) {
				return false;
			}
			break;
		case 9: // oct
			if (day > 31) {
				return false;
			}
			break;
		case 10: // nov
			if (day > 30) {
				return false;
			}
			break;
		case 11: // dec
			if (day > 31) {
				return false;
			}
			break;
		default:
			break;
		}
		t.tm_mday = static_cast<int>(day);
		now = mktime(&t);
		update_date_component_if_possible();
		return true;
	}
	inline bool set_month(uint8_t month) {
		month -= 1;
		if (month > 11) {
			return false;
		}
		time_t local_now = now;
		struct tm t;
		localtime_s(&t, &local_now);
		t.tm_mon = static_cast<int>(month);
		now = mktime(&t);
		update_date_component_if_possible();
		return true;
	}
	inline bool set_year(unsigned int year) {
		time_t local_now = now;
		struct tm t;
		localtime_s(&t, &local_now);
		t.tm_year = static_cast<int>(year);
		now = mktime(&t);
		update_date_component_if_possible();
		return true;
	}
};
