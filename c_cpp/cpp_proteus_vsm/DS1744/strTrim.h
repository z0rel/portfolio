#pragma once


#include <string>

/// Функция удаления начальных и конечных пробелов 
std::string strTrim(std::string str)
{
	std::string sTmp = str;

	// Первый символ, не являющийся пробелом
	int iFirst = sTmp.find_first_not_of(" ");

	// Если непустая строка, то
	if (iFirst != std::string::npos)
	{
		// Удалить начальные пробелы
		sTmp = sTmp.substr(iFirst, sTmp.length() - iFirst);
		// Последний символ, не являющийся пробелом
		iFirst = sTmp.find_last_not_of(" ");
		// Удалить конечные пробелы
		sTmp = sTmp.substr(0, iFirst + 1);
		return sTmp; // Возврат результата
	}
	else
	{
		return ""; // На входе была строка из одних пробелов
	}
}
