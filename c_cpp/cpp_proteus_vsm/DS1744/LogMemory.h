#pragma once

#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <limits>
#include <stdint.h>
#include <numeric>

#include "StepState.h"

class LogMemory
{
	unsigned int prev_addr = UINT_MAX;
	unsigned int prev_value = UINT_MAX;
	unsigned int prev_is_read = UINT_MAX;
public:

	LogMemory();
	~LogMemory();
	void save_file();
	void push_back(const StepState& state);

	bool disable = false;
	std::vector<StepState> logMemory;
	std::ofstream str;
};

