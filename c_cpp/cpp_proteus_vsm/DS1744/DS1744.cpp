#include "pch.h" // заголовочный файл предкомпиляции заголовков (ВЕРХНИЙ).

// #include <boost/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <limits>

#include "DS1744.h"
#include "ProteusLogger.h"


/// Словарь сообщений (для InfoLOG)
std::map<INT, std::string> dicLOG;

/// Буфер заголовка инфо сообщения (для InfoLOG)
CHAR chBuf[200];

/// Счетчик сообщений (для InfoLOG)
INT RowCount;

/// Глобальный экземпляр интерфейса модели (для InfoLOG)
IINSTANCE* gInst;

/// Флаг выдачи буф. сообщ. в лог симулятора (для InfoLOG)
BOOL FlushFlag = FALSE;


DS1744::DS1744(const char* device)
{
	// InfoLOG("=>> TypeModel::TypeModel => Model: %20s" " => Class CONSTRUCTOR", device);
}


DS1744::~DS1744() {}


INT DS1744::isdigital(CHAR* pinname)
{
	// InfoLOG("=>> TypeModel::isdigital => DSIM Model => PIN Name = %4s", pinname);
	return TRUE;
}


VOID DS1744::setup(IINSTANCE* _inst, IDSIMCKT* dsim)
{
	this->vsm_model = _inst;
	gInst = this->vsm_model; // Для правильной работы InfoLOG
	dCkt = dsim;
	dCkt->systime(&DSIMTime);
	// InfoLOG("=>> TypeModel::setup => DSIM Model" " => TIME=%.12f", realtime(DSIMTime));
	pin_manager.setup_pins(_inst, dsim);
}


VOID DS1744::runctrl(RUNMODES mode)
{
	if (mode == RM_START) // Режим начального запуска. setup еще не отработал
	{
		// InfoLOG("=>> TypeModel::runctrl => DSIM Model => RUNMODES =%2i", mode);
	}
	else
	{
		// dCkt->systime(&DSIMTime);
		// InfoLOG("=>> TypeModel::runctrl => DSIM Model => RUNMODES =%2i => TIME=%.12f", mode, realtime(DSIMTime));
	}
}


VOID DS1744::actuate(REALTIME time, ACTIVESTATE newstate)
{
	// InfoLOG("=>> TypeModel::actuate => DSIM Model => ACTIVESTATE =%2li => TIME=%.12f", newstate, time);
}


BOOL DS1744::indicate(REALTIME time, ACTIVEDATA* data)
{
	// InfoLOG("=>> TypeModel::indicate => DSIM Model => TIME=%.12f", time);
	return FALSE;
}


VOID DS1744::simulate(ABSTIME time, DSIMMODES mode)
{
	StepState state;
	this->pin_manager.read_state(state);
	if (!state.isError() && !state.ce) {
		state.memory_data = this->memory.read(state.memory_address, state.input_data);
	}

	// this->pin_manager.disable = true;
	// this->memory.log.disable = true;

	// this->memory.log.push_back(state);

	if (state.isError()) {
		return;
	}
	// this->memory.log.disable = true;

	if (state.ce) { // перевести все выходы в strong undefined
		this->pin_manager.set_output_d_pins_tristate();
	}
	else {
		if (!state.we) {
			// ---- Записать данные из D в память --
			this->memory.write(state.memory_address, state.input_data);
		}
		else if (!state.oe) {
			// ---- Вывести данные из памяти на D ---
			if (state.edge_state_oe) {
				this->pin_manager.set_output_d_pins(state.memory_data);
			}
		}
		else if (state.oe && state.edge_state_oe) {
			this->pin_manager.set_output_d_pins_weak_high();
		}
	}
}


VOID DS1744::callback(ABSTIME time, EVENTID eventid)
{
	//InfoLOG("=>> TypeModel::callback => DSIM Model => EVENTID =%2li => TIME=%.12f",
	// eventid, realtime(time));
}


/// Полная перерисовка окон. Для animate это окно Editing Window
VOID DS1744::ReDrawScreen()
{
	static HWND hWnd = NULL;
	if (hWnd == NULL)
	{
		hWnd = GetFocus();
	}
	InvalidateRect(hWnd, NULL, TRUE);
}


/// Функция перевода ms в ps для DSIM-симуляции 
ABSTIME DS1744::mS(double val) { return (ABSTIME)(val * 1000000000); }


/// Функция перевода us в ps для DSIM-симуляции 
ABSTIME DS1744::uS(double val) { return (ABSTIME)(val * 1000000); }


/// Функция перевода ns в ps для DSIM-симуляции 
ABSTIME DS1744::nS(double val) { return (ABSTIME)(val * 1000); }




	// if (++this->count_of_print < 1000) {
	// 	msgAS("OE=%d, WE=%d, CE=%d, a=%X", isOE, isWE, isCE, memory_address);
	// }

