#include "pch.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <limits>
#include <utility>

#include <fmt/core.h>
#include "LogMemory.h"

#include "ProteusLogger.h"
#include "ds1744_common.h"


LogMemory::LogMemory()
	: str("J:\\devs\\vvn8\\vsm_models\\DS1744\\Debug\\log\\operations.log")
{
	if (disable) {
		return;
	}
	str << "{" << std::endl << "  \"objects\": [" << std::hex;
}


LogMemory::~LogMemory()
{
	if (disable) {
		return;
	}
	str << std::endl << "    {}" << std::endl << "  }" << std::endl << "}";
	str.close();
}


void LogMemory::push_back(const StepState& obj)
{
	if (disable) {
		return;
	}
	str << fmt::format(
		"{}{ {}\"a\": \"{}\"{}{}{}, \"di\": \"{}\"{}, \"dm\": \"{}\"{}, \"we\": {}, \"oe\": {}, \"swe\": {}{},\"soe\": {}{}, \"sa\": [{}{},{}{},{}{},{}{},{}{},{}{},{}{},{}{},{}{},{}{},{}{},{}{},{}{},{}{},{}{}], \"sd\": [{}{},{}{},{}{},{}{},{}{},{}{},{}{},{}{}] },",
		!(prev_addr == obj.memory_address && prev_is_read == static_cast<unsigned int>(obj.we)) ? "\n   " : "",
		obj.we ? "\"read\" , " : "\"write\", ",
		static_cast<unsigned int>(obj.memory_address),
		obj.memory_address < 0x10 ? " " : "",
		obj.memory_address < 0x100 ? " " : "",
		obj.memory_address < 0x1000 ? " " : "",
		static_cast<unsigned int>(obj.input_data),
		obj.input_data < 0x10 ? " " : "",
		static_cast<unsigned int>(obj.memory_data),
		obj.memory_data < 0x10 ? " " : "",
		static_cast<int>(obj.we),
		static_cast<int>(obj.oe),
		stateToStringShort(obj.state_we),
		(obj.edge_state_we ? "/" : " "),
		stateToStringShort(obj.state_oe),
		(obj.edge_state_oe ? "/" : " "),
		stateToStringShort(obj.state_address[0]), (obj.edge_state_address[0] ? "/" : " "),
		stateToStringShort(obj.state_address[1]), (obj.edge_state_address[1] ? "/" : " "),
		stateToStringShort(obj.state_address[2]), (obj.edge_state_address[2] ? "/" : " "),
		stateToStringShort(obj.state_address[3]), (obj.edge_state_address[3] ? "/" : " "),
		stateToStringShort(obj.state_address[4]), (obj.edge_state_address[4] ? "/" : " "),
		stateToStringShort(obj.state_address[5]), (obj.edge_state_address[5] ? "/" : " "),
		stateToStringShort(obj.state_address[6]), (obj.edge_state_address[6] ? "/" : " "),
		stateToStringShort(obj.state_address[7]), (obj.edge_state_address[7] ? "/" : " "),
		stateToStringShort(obj.state_address[8]), (obj.edge_state_address[8] ? "/" : " "),
		stateToStringShort(obj.state_address[9]), (obj.edge_state_address[9] ? "/" : " "),
		stateToStringShort(obj.state_address[10]), (obj.edge_state_address[10] ? "/" : " "),
		stateToStringShort(obj.state_address[11]), (obj.edge_state_address[11] ? "/" : " "),
		stateToStringShort(obj.state_address[12]), (obj.edge_state_address[12] ? "/" : " "),
		stateToStringShort(obj.state_address[13]), (obj.edge_state_address[13] ? "/" : " "),
		stateToStringShort(obj.state_address[14]), (obj.edge_state_address[14] ? "/" : " "),
		stateToStringShort(obj.state_data[0]), (obj.edge_state_data[0] ? "/" : " "),
		stateToStringShort(obj.state_data[1]), (obj.edge_state_data[1] ? "/" : " "),
		stateToStringShort(obj.state_data[2]), (obj.edge_state_data[2] ? "/" : " "),
		stateToStringShort(obj.state_data[3]), (obj.edge_state_data[3] ? "/" : " "),
		stateToStringShort(obj.state_data[4]), (obj.edge_state_data[4] ? "/" : " "),
		stateToStringShort(obj.state_data[5]), (obj.edge_state_data[5] ? "/" : " "),
		stateToStringShort(obj.state_data[6]), (obj.edge_state_data[6] ? "/" : " "),
		stateToStringShort(obj.state_data[7]), (obj.edge_state_data[7] ? "/" : " ")
	);
	
	this->prev_addr = obj.memory_address;
	this->prev_value = obj.input_data;
	this->prev_is_read = obj.isRead;
};


void LogMemory::save_file() {}

