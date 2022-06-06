#include "pch.h"
#include "MemoryRtc.h"

MemoryRtc::MemoryRtc() {

}


uint8_t MemoryRtc::read(uint16_t address, uint8_t data)
{
	uint8_t output_data;
	uint8_t t_output_data = this->memory[address];
	switch (address) {
		case 0x7FFF: {
			// |  B7   | B6   B5 |   B4    | B3  | B2 B1 B0  |   Function  Range
			// |        10 Year            |     Year        |   Year      00-99
			output_data = TimeTracker::to_bcd(this->current_time.year());
			break;
		}
		case 0x7FFE: {
			// |  B7   | B6   B5 |    B4    | B3  | B2 B1 B0 |
			// |   X   |  X    X | 10 Month |    Month       |   Month 01-12
			output_data = TimeTracker::to_bcd(this->current_time.month()) | (t_output_data & ((1 << 7) | (1 << 6) | (1 << 5)));
			break;
		}
		case 0x7FFD: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0 |
			// |   X   |  X |    10 Date    |    Date        |   Date 01-31
			output_data = TimeTracker::to_bcd(this->current_time.day()) | (t_output_data & ((1 << 7) | (1 << 6)));
			break;
		}
		case 0x7FFC: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0 |
			// |  BF   | FT |  X |     X    |  X  |   Day    |   Day 01-07
			// FT = Frequency Test
			// BF = Battery Flag

			unsigned int value_day_week = this->current_time.day_week();
			bool ft = this->current_time.frequency_test_bit();
			bool bf = this->current_time.battary_flag_bit();
			output_data = (bf ? (1 << 7) : 0) | (ft ? (1 << 6) : 0) | static_cast<uint8_t>(value_day_week) | (t_output_data & ((1 << 5) | (1 << 4) | (1 << 3)));
			break;
		}
		case 0x7FFB: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0 |
			// |   X      X |    10 Hour    |    Hour        |   Hour 00-23
			output_data = TimeTracker::to_bcd(this->current_time.hour()) | (t_output_data & ((1 << 7) | (1 << 6)));
			break;
		}
		case 0x7FFA: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0 |
			// |   X   |    10 Minutes      |    Minutes     |   Minutes 00-59
			output_data = TimeTracker::to_bcd(this->current_time.minute()) | (t_output_data & (1 << 7));
			break;
		}
		case 0x7FF9: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0
			// | #OSC  |    10 Seconds      |    Seconds     |   Seconds 00-59
			// #OSC = oscillator bit (stop bit)
			output_data = (this->current_time.osc_bit() ? (1 << 7) : 0) | TimeTracker::to_bcd(this->current_time.second());
			break;
		}
		case 0x7FF8: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0 |
			// |   W   |  R |   10 Century  |    Century     |   Century 00-39
			// W = Write Bit
			// R = Read Bit
			bool wb = this->current_time.write_bit();
			bool rb = this->current_time.read_bit();
			output_data = (wb ? (1 << 7) : 0) | (rb ? (1 << 7) : 0) | TimeTracker::to_bcd(this->current_time.year() / 100 + 19);
			break;
		}
	default:
		output_data = this->memory[address];
		break;
	}
	return output_data;
}


void MemoryRtc::write(uint16_t address, uint8_t data)
{
	switch (address) {
		case 0x7FFF: {
			// |  B7   | B6   B5 |   B4    | B3  | B2 B1 B0  |   Function  Range
			// |        10 Year            |     Year        |   Year      00-99
			uint8_t year = TimeTracker::from_bcd(data);
			this->current_time.set_year((this->current_time.year() / 100) * 100 + year);
			break;
		}
		case 0x7FFE: {
			// |  B7   | B6   B5 |    B4    | B3  | B2 B1 B0 |
			// |   X   |  X    X | 10 Month |    Month       |   Month 01-12
			uint8_t month = TimeTracker::from_bcd(data & 0x1F);
			this->current_time.set_month(month);
			break;
		}
		case 0x7FFD: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0 |
			// |   X   |  X |    10 Date    |    Date        |   Date 01-31
			uint8_t day = TimeTracker::from_bcd(data & 0x3F);
			this->current_time.set_day(day);
			break;
		}
		case 0x7FFC: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0 |
			// |  BF   | FT |  X |     X    |  X  |   Day    |   Day 01-07
			// FT = Frequency Test
			// BF = Battery Flag (Read Only)
			uint8_t day_week = data & 3;
			bool ft = (data & (1 << 7)) ? true : false;
			this->current_time.set_frequency_test_bit(ft);
			break;
		}
		case 0x7FFB: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0 |
			// |   X      X |    10 Hour    |    Hour        |   Hour 00-23
			uint8_t hour = TimeTracker::from_bcd(data & 0x3F);
			this->current_time.set_hour(hour);
			break;
		}
		case 0x7FFA: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0 |
			// |   X   |    10 Minutes      |    Minutes     |   Minutes 00-59
			uint8_t minute = TimeTracker::from_bcd(data & 0x7F);
			this->current_time.set_minute(minute);
			break;
		}
		case 0x7FF9: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0
			// | #OSC  |    10 Seconds      |    Seconds     |   Seconds 00-59
			// #OSC = oscillator bit (stop bit)
			uint8_t second = TimeTracker::from_bcd(data) & 0x7F;
			bool _osc = (data & (1 << 7)) ? true : false;
			this->current_time.set_second(second);
			this->current_time.set_osc_bit(_osc);
			break;
		}
		case 0x7FF8: {
			// |  B7   | B6 | B5 |    B4    | B3  | B2 B1 B0 |
			// |   W   |  R |   10 Century  |    Century     |   Century 00-39
			// W = Write Bit
			// R = Read Bit
			bool wb = (data & (1 << 7)) ? true : false;
			bool rb = (data & (1 << 6)) ? true : false;
			uint8_t century = TimeTracker::from_bcd(data & 0x3F) - 19;
			this->current_time.set_read_bit(rb);
			this->current_time.set_write_bit(wb);
			this->current_time.set_year((this->current_time.year() % 100) + century * 100);
			break;
		}
	default:
		break;
	}

	this->memory[address] = data;
}
