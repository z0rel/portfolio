#pragma once

#include "vsm.h"

#include "StepState.h"

class PinManager
{
	void _read_memory_address(StepState& state);
	void _read_data(StepState& state);
	bool _read_bool_val(StepState& state, STATE st);
	bool _read_flt_val(StepState& state, STATE st);
public:
	void setup_pins(IINSTANCE* _inst, IDSIMCKT* dsim);
	uint16_t memory_address();

	void read_state(StepState& state);
	void set_output_d_pins(uint8_t output_data);
	void set_output_d_pins_tristate();
	void set_output_d_pins_weak_high();

	bool disable = false;


	IDSIMPIN* pins_A_0_to_14[15] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	IDSIMPIN* pins_D_0_to_7[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	IDSIMPIN* pin__CE = nullptr;
	IDSIMPIN* pin__WE = nullptr;
	IDSIMPIN* pin__OE = nullptr;
};

