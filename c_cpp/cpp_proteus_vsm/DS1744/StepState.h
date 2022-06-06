#pragma once

#include <bitset>
#include "stdint.h"

#include "ds1744_common.h"


struct StepStateBase {
	uint16_t memory_address;
	uint8_t input_data;
	uint8_t memory_data;

	STATE state_ce;
	STATE state_oe;
	STATE state_we;
	STATE state_address[15];
	STATE state_data[8];

	bool isRead:1;
	bool ce:1;
	bool we:1;
	bool oe:1;
	bool edge_state_ce:1;
	bool edge_state_oe:1;
	bool edge_state_we:1;

	bool badOperation:1;
	bool isFloatStateMemoryAddreess:1;
	bool isFloatStateCE:1;
	bool isFloatStateWE:1;
	bool isFloatStateOE:1;
	bool isFloatStateInputData:1;

	std::bitset<15> edge_state_address;
	std::bitset<8> edge_state_data;

	void clear() {
		memory_address = 0;
		input_data = 0;
		memory_data = 0;
		isRead = false;
		ce = false;
		we = false;
		oe = false;
		badOperation = false;
		isFloatStateMemoryAddreess = false;
		isFloatStateCE = false;
		isFloatStateWE = false;
		isFloatStateOE = false;
		isFloatStateInputData = false;
		state_ce = STATE::UNDEFINED;
		state_oe = STATE::UNDEFINED;
		state_we = STATE::UNDEFINED;
		edge_state_ce = false;
		edge_state_oe = false;
		edge_state_we = false;
		edge_state_address.reset();
		edge_state_data.reset();
		for (int i = 0; i < SIZEOF_ARRAY(state_address); ++i) {
			state_address[i] = STATE::UNDEFINED;
		}
		for (int i = 0; i < SIZEOF_ARRAY(state_data); ++i) {
			state_data[i] = STATE::UNDEFINED;
		}
	}
};



class StepState : public StepStateBase
{
public:
	void clear() 
	{
		StepStateBase::clear();
	}

	bool isError() const 
	{
		return this->isFloatStateMemoryAddreess || this->isFloatStateCE || this->isFloatStateWE || this->isFloatStateOE || this->memory_address >= 32768;
	}
};

