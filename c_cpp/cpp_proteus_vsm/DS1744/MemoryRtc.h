#pragma once

#include <vector>

#include "LogMemory.h"
#include "TimeTracker.h"


class MemoryRtc
{
	std::vector<uint8_t> memory = std::vector<uint8_t>(32768, 0);
	// Текущее время
	TimeTracker current_time;
public:
	MemoryRtc();

	uint8_t read(uint16_t address, uint8_t data);
	void write(uint16_t address, uint8_t data);

	LogMemory log;
};

