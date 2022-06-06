// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "DS1744.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


// Цифровая модель
extern "C" __declspec(dllexport) IDSIMMODEL* createdsimmodel(CHAR *device, ILICENCESERVER *ils)
{
	// msgAS("IDSIMMODEL *createdsimmodel\nDSIM Модель: %s", device);
	// InfoLOG("=>> IDSIMMODEL createdsimmodel => DSIM Model: %20s", device);
	if (ils->authorize(TypeModel_Key))
	{
		// msgAS("IDSIMMODEL *createdsimmodel\nDSIM Модель: %s\nПрошла авторизацию!", device);
		// InfoLOG("=>> IDSIMMODEL createdsimmodel => DSIM Model: %20s"
		// " => Authorization OK!", device);
		// TypeModel *newModel = new TypeModel(TypeModel_Name);
		// return newModel; // указатель на класс модели
		return new DS1744(TypeModel_Name); // указатель на класс модели
	}
	else
	{
		// msgAS("IDSIMMODEL *createdsimmodel\nDSIM Модель: %s\nОшибка авторизации!", device);
		// InfoLOG("=>> IDSIMMODEL createdsimmodel => DSIM Model: %20s"
		// " => Authorization ERROR", device);
		return new DS1744(TypeModel_Name); // указатель на класс модели
		return FALSE; // Авторизация не прошла
	}
}
// ------------------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) VOID deletedsimmodel(IDSIMMODEL * model)
{
	// msgAS("VOID deletedsimmodel\nВыполняется удаление DSIM модели!");
	delete (DS1744*)model;
}
