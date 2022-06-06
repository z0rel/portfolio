#include "pch.h"
#include "PinManager.h"
#include "ds1744_common.h"
#include "ProteusLogger.h"


void PinManager::setup_pins(IINSTANCE* _inst, IDSIMCKT* dsim)
{
	this->pins_A_0_to_14[0] = _inst->getdsimpin(const_cast<char*>("A0"), TRUE);
	this->pins_A_0_to_14[1] = _inst->getdsimpin(const_cast<char*>("A1"), TRUE);
	this->pins_A_0_to_14[2] = _inst->getdsimpin(const_cast<char*>("A2"), TRUE);
	this->pins_A_0_to_14[3] = _inst->getdsimpin(const_cast<char*>("A3"), TRUE);
	this->pins_A_0_to_14[4] = _inst->getdsimpin(const_cast<char*>("A4"), TRUE);
	this->pins_A_0_to_14[5] = _inst->getdsimpin(const_cast<char*>("A5"), TRUE);
	this->pins_A_0_to_14[6] = _inst->getdsimpin(const_cast<char*>("A6"), TRUE);
	this->pins_A_0_to_14[7] = _inst->getdsimpin(const_cast<char*>("A7"), TRUE);
	this->pins_A_0_to_14[8] = _inst->getdsimpin(const_cast<char*>("A8"), TRUE);
	this->pins_A_0_to_14[9] = _inst->getdsimpin(const_cast<char*>("A9"), TRUE);
	this->pins_A_0_to_14[10] = _inst->getdsimpin(const_cast<char*>("A10"), TRUE);
	this->pins_A_0_to_14[11] = _inst->getdsimpin(const_cast<char*>("A11"), TRUE);
	this->pins_A_0_to_14[12] = _inst->getdsimpin(const_cast<char*>("A12"), TRUE);
	this->pins_A_0_to_14[13] = _inst->getdsimpin(const_cast<char*>("A13"), TRUE);
	this->pins_A_0_to_14[14] = _inst->getdsimpin(const_cast<char*>("A14"), TRUE);

	this->pins_D_0_to_7[0] = _inst->getdsimpin(const_cast<char*>("D0"), TRUE);
	this->pins_D_0_to_7[1] = _inst->getdsimpin(const_cast<char*>("D1"), TRUE);
	this->pins_D_0_to_7[2] = _inst->getdsimpin(const_cast<char*>("D2"), TRUE);
	this->pins_D_0_to_7[3] = _inst->getdsimpin(const_cast<char*>("D3"), TRUE);
	this->pins_D_0_to_7[4] = _inst->getdsimpin(const_cast<char*>("D4"), TRUE);
	this->pins_D_0_to_7[5] = _inst->getdsimpin(const_cast<char*>("D5"), TRUE);
	this->pins_D_0_to_7[6] = _inst->getdsimpin(const_cast<char*>("D6"), TRUE);
	this->pins_D_0_to_7[7] = _inst->getdsimpin(const_cast<char*>("D7"), TRUE);

	this->pin__CE = _inst->getdsimpin(const_cast<char*>("$CE$"), TRUE);
	this->pin__WE = _inst->getdsimpin(const_cast<char*>("$WE$"), TRUE);
	this->pin__OE = _inst->getdsimpin(const_cast<char*>("$OE$"), TRUE);
}


void PinManager::read_state(StepState& state)
{
	state.clear();
	// if (++this->count_of_print < 1000) {
	// 	InfoLOG("OE=%s, WE=%s, CE=%s, a=%i", STATEs[static_cast<int>(st_OE)], STATEs[static_cast<int>(st_WE)], STATEs[static_cast<int>(st_CE)]);
	// }
	this->_read_memory_address(state);

	state.state_ce = this->pin__CE->istate();
	state.state_oe = this->pin__OE->istate();
	state.state_we = this->pin__WE->istate();

	state.edge_state_ce = this->pin__CE->isedge();
	state.edge_state_oe = this->pin__OE->isedge();
	state.edge_state_we = this->pin__WE->isedge();

	state.isFloatStateOE = this->_read_flt_val(state, state.state_oe);
	state.oe = this->_read_bool_val(state, state.state_oe);
	
	state.isFloatStateWE = this->_read_flt_val(state, state.state_we);
	state.we = this->_read_bool_val(state, state.state_we);

	state.isFloatStateCE = this->_read_flt_val(state, state.state_ce);
	state.ce = this->_read_bool_val(state, state.state_ce);
	this->_read_data(state);
}


void PinManager::set_output_d_pins(uint8_t output_data)
{
	if (disable) {
		return;
	}
	// this->logMemory.push_back(LogMemory{ memory_address, input_data, true, isCE, isWE, isOE });
	// InfoLOG("read : add[%X]=>%X", memory_address, output_data);

	for (int i = 0; i < SIZEOF_ARRAY(this->pins_D_0_to_7); ++i) {
		STATE st = this->pins_D_0_to_7[i]->istate();
		if (((1 << i) & output_data) != 0) { // бит установлен
			if (st != STATE::WHI && st != STATE::SHI && st != STATE::IHI && st != STATE::PHI) {
				this->pins_D_0_to_7[i]->drivestate(0, STATE::SHI);
			}
		}
		else { // бит не установлен
			if (st != STATE::WLO && st != STATE::SLO && st != STATE::ILO && st != STATE::PLO) {
				this->pins_D_0_to_7[i]->drivestate(0, STATE::SLO);
			}
		}
	}
}


void PinManager::set_output_d_pins_tristate()
{
	if (disable) {
		return;
	}

	for (int i = 0; i < SIZEOF_ARRAY(this->pins_D_0_to_7); ++i) {
		this->pins_D_0_to_7[i]->drivestate(0, STATE::WUD);
	}
}

void PinManager::set_output_d_pins_weak_high() {
	if (disable) {
		return;
	}

	for (int i = 0; i < SIZEOF_ARRAY(this->pins_D_0_to_7); ++i) {
		this->pins_D_0_to_7[i]->drivestate(0, STATE::WHI);
	}
}

void PinManager::_read_memory_address(StepState& state)
{
	for (int i = 0; i < SIZEOF_ARRAY(this->pins_A_0_to_14); ++i) {
		STATE st = this->pins_A_0_to_14[i]->istate();
		state.state_address[i] = st;
		state.edge_state_address[i] = this->pins_A_0_to_14[i]->isedge();

		switch (st) {
		case STATE::WHI:
		case STATE::SHI:
		case STATE::IHI:
		case STATE::PHI:
			state.memory_address |= (1 << i);
			break;

		case STATE::PLO:
		case STATE::ILO:
		case STATE::SLO:
		case STATE::WLO:
			break;

		case STATE::FLT:
		case STATE::WUD:
		case STATE::SUD:
			// InfoLOG("A[i]=%i, state=%s, a=%X", i, stateToString(st), state.memory_address);
			state.isFloatStateMemoryAddreess = true;
			break;
		default:
			state.isFloatStateMemoryAddreess = true;
			break;
		}
	}
}


void PinManager::_read_data(StepState& state)
{
	for (int i = 0; i < SIZEOF_ARRAY(this->pins_D_0_to_7); ++i) {
		STATE st = this->pins_D_0_to_7[i]->istate();
		state.state_data[i] = st;
		state.edge_state_data[i] = this->pins_D_0_to_7[i]->isedge();

		switch (st) {
		case STATE::WHI:
		case STATE::SHI:
		case STATE::IHI:
		case STATE::PHI:
			state.input_data |= (1 << i);
			break;
		case STATE::PLO:
		case STATE::ILO:
		case STATE::SLO:
		case STATE::WLO:
			break;
		case STATE::FLT:
		case STATE::WUD:
		case STATE::SUD:
			state.isFloatStateInputData = true;
			break;
		default:
			state.isFloatStateInputData = true;
			break;
		}
	}
}


bool PinManager::_read_bool_val(StepState& state, STATE st)
{
	switch (st) {
	case STATE::WHI:
	case STATE::SHI:
	case STATE::IHI:
	case STATE::PHI:
		return true;
	case STATE::PLO:
	case STATE::ILO:
	case STATE::SLO:
	case STATE::WLO:
		return false;
	case STATE::FLT:
	case STATE::WUD:
	case STATE::SUD:
		return false;
	default:
		return false;
	}
}


bool PinManager::_read_flt_val(StepState& state, STATE st)
{
	switch (st) {
	case STATE::WHI:
	case STATE::SHI:
	case STATE::IHI:
	case STATE::PHI:
		return false;
	case STATE::PLO:
	case STATE::ILO:
	case STATE::SLO:
	case STATE::WLO:
		return false;
	case STATE::FLT:
	case STATE::WUD:
	case STATE::SUD:
		return true;
	default:
		return true;
	}
}
