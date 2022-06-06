#pragma once

#include "vsm.h"
#include <string>
#include <map>


// Глобальный экземпляр интерфейса модели (для InfoLOG)
extern IINSTANCE* gInst;

// Флаг выдачи буфера сообщ. в лог симулятора (для InfoLOG)
extern BOOL FlushFlag;

// Буфер заголовка инфо сообщения (для InfoLOG)
extern CHAR chBuf[200];

// Счетчик сообщений (для InfoLOG)
extern INT RowCount;

// Словарь сообщений (для InfoLOG)
extern std::map<INT, std::string> dicLOG;


/// Функция вывода информационного сообщения на экран 
template<typename... Args> void msgAS(const Args&... args)
{
	// Словарь экранных инф. сообщ. <KEY:STRING, VALUE:INT>
	static std::map<std::string, INT> Dict;

	CHAR strBuffer[200];
	sprintf_s(strBuffer, args...);

	std::string sDict = strBuffer;
	INT iTmp = sDict.find_first_of("\n");
	sDict = sDict.substr(0, iTmp);
	auto Result = Dict.find(sDict);

	// Если MessageBox не найден в словаре, то
	if (Result == Dict.end())
	{
		// "Выводить..." ДА/НЕТ
		std::string sMsg = strBuffer;
		sMsg += "\n\n\nВыводить это сообщение в следующий раз?";
		INT msgID = MessageBoxA(0, sMsg.c_str(), "ОТЛАДОЧНАЯ ИНФОРМАЦИЯ", MB_ICONINFORMATION | MB_YESNO);

		// Если больше не выводить данный MessageBox, то
		if (msgID == IDNO)
		{
			// Добавить в словарь
			Dict.insert(std::pair<std::string, INT>(sDict, 1));
		}
	}
}

/// Буфферизация информационных сообщений до появления экземпляра интерфейса модели 
template<typename... Args> void InfoLOG(const Args&... args)
{
	// Ни один из setup еще не отработал - накопление сообщений
	if (gInst == NULL)
	{
		RowCount += 1;
		sprintf_s(chBuf, args...);
		std::string strTmp = chBuf;
		dicLOG.insert(std::pair<INT, std::string>(RowCount, strTmp));
	}
	else // Отработал setup.
	{
		// Буфер сообщений еще не выдавался в лог симулятора
		if (!FlushFlag)
		{
			// Выдать буфер в лог симулятора
			for (auto& p : dicLOG)
			{
				gInst->log(_strdup(p.second.c_str()));
			}
			FlushFlag = TRUE;
		}
		// Дальнейший вывод через интерфейс модели
		sprintf_s(chBuf, args...);
		gInst->log(chBuf);
	}
}


// Функция удаления начальных и конечных пробелов
std::string strTrim(std::string str);




inline const char* stateToString(STATE st) {
	switch (st) {
	case STATE::UNDEFINED:
		return "UNDEFINED = 0";
	case STATE::TSTATE:
		return "TSTATE = 1";
	case STATE::FSTATE:
		return "FSTATE = -1";
	case STATE::PLO:
		return "PLO = SS_POWER + SP_LOW = 29";
	case STATE::ILO:
		return "ILO = SS_INJECT + SP_LOW = 21";
	case STATE::SLO:
		return "SLO = SS_STRONG + SP_LOW = 13";
	case STATE::WLO:
		return  "WLO = SS_WEAK + SP_LOW = 5";
	case STATE::FLT:
		return  "FLT = SS_FLOAT + SP_FLOAT = 2";
	case STATE::WHI:
		return  "WHI = SS_WEAK + SP_HIGH = 7";
	case STATE::SHI:
		return  "SHI = SS_STRONG + SP_HIGH = 15";
	case STATE::IHI:
		return  "IHI = SS_INJECT + SP_HIGH = 23";
	case STATE::PHI:
		return  "PHI = SS_POWER + SP_HIGH = 31";
	case STATE::WUD:
		return  "WUD = SS_WEAK + SP_UNDEFINED = 4";
	case STATE::SUD:
		return  "SUD = SS_STRONG + SP_UNDEFINED = 12";
	default:
		return "";
	}
	return "";
}


inline const char* stateToStringShort(STATE st) {
	switch (st) {
	case STATE::UNDEFINED:
		return "UN";
	case STATE::TSTATE:
		return "TS";
	case STATE::FSTATE:
		return "FS";
	case STATE::PLO:
		return "PL";
	case STATE::ILO:
		return "IL";
	case STATE::SLO:
		return "SL";
	case STATE::WLO:
		return "WL";
	case STATE::FLT:
		return "FL";
	case STATE::WHI:
		return "WH";
	case STATE::SHI:
		return "SH";
	case STATE::IHI:
		return "IH";
	case STATE::PHI:
		return "PH";
	case STATE::WUD:
		return "WU";
	case STATE::SUD:
		return "SU";
	default:
		return "";
	}
	return "";
}

