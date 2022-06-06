#pragma once

#include "LogMemory.h"
#include "MemoryRtc.h"
#include "PinManager.h"
#include "TimeTracker.h"


// Product ID value obtained from Labcenter
#define TypeModel_Key 0x00000000 

// Имя модели
#define TypeModel_Name "TypeModel" 


class DS1744 : public IDSIMMODEL {
public:
	DS1744(const char* device);
	~DS1744();
 
	// Переопределяемые функции DSIM Модели
	INT isdigital(CHAR* pinname);
	VOID setup(   IINSTANCE* inst, IDSIMCKT* dsim);
	VOID runctrl( RUNMODES mode);
	VOID actuate( REALTIME time, ACTIVESTATE newstate);
	BOOL indicate(REALTIME time, ACTIVEDATA* data);
	VOID simulate(ABSTIME  time, DSIMMODES mode);
	VOID callback(ABSTIME  time, EVENTID eventid);

	/// Перерисовка окна Editing Window
	VOID ReDrawScreen();

	/// Перевод ms в ps для DSIM-симуляции
	ABSTIME mS(double val);

	/// Перевод us в ps для DSIM-симуляции
	ABSTIME uS(double val);

	/// Перевод ns в ps для DSIM-симуляции
	ABSTIME nS(double val);

private:
	/// Экземпляр интерфейса модели
	IINSTANCE* vsm_model = nullptr;

	/// Экземпляр цифрового интерфейса модели
	IDSIMCKT* dCkt = nullptr;

	/// Экземпляр аналогового интерфейса модели
	ISPICECKT* aCkt = nullptr;

	/// Экземпляр интерактивного интерфейса модели
	ICOMPONENT* Comp = nullptr;

	/// Текущее DSIM время, относительно старта симуляции
	ABSTIME DSIMTime = 0;

	/// Текущее SPICE время, относительно старта симуляции
	DOUBLE SPICETime = 0.0;

    // Экземпляры контактов A0-A14



	PinManager pin_manager;

	MemoryRtc memory;

	// /// Структура для POPUP-окон
	// CREATEPOPUPSTRUCT *cps; 

	// /// Окно IUSERPOPUP
	// IUSERPOPUP *PopupUser; 

	// /// Окно IDEBUGPOPUP
	// IDEBUGPOPUP *PopupDebug; 

	// /// Окно ISTATUSPOPUP
	// ISTATUSPOPUP *PopupStatus; 

	// /// Окно IMEMORYPOPUP
	// IMEMORYPOPUP *PopupMemory; 

	int count_of_print = 0;
};
