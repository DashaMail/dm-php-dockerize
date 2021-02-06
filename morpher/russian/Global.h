#pragma once
#include <map>
#include <sstream>
#include "../tstring.h"
#include "../tchar.h"
#include "Indirect.h"

enum RodChislo // C#: РодЧисло
{
	SingMasc,
	SingFem,
	SingNeut,
	Plural,
	RodChislo_Count
};

enum Padeg // C#: Падеж
{
	Padeg_I,
	Padeg_R,
	Padeg_D,
	Padeg_V,
	Padeg_T,
	Padeg_P,
	Padeg_M
};

enum Attributes // C#: Признак
{
	Attribute_Masc = 1,
	Attribute_Fem = 2,
	Attribute_Fio = 4,
	Attribute_Naric = 8,
	Attribute_Toponym = 16
};

enum EndingsType // C#: ТипСклонения
{
	EndingsType_NounLike,    // Субстантивный
	EndingsType_Adjectival,  // Адъективный
	EndingsType_Uninflected, // Несклоняемое
	EndingsType_Pronomial    // Местоимённый
};

enum FioPart
{
	FioPart_FamilyName, // Фамилия
	FioPart_GivenName,  // Имя
	FioPart_Patronymic  // Отчество
};

enum Gender
{
	Gender_Masculine,
	Gender_Feminine,
	Gender_Unisex
};

struct Suffix
{
	std::tstring Morphs;
	FioPart fioPart;
	Gender gender;
};

static Suffix suffixes[] =
{
	// Порядок перечисления важен.
	{ _T("ович"), FioPart_Patronymic, Gender_Masculine },
	{ _T("овна"), FioPart_Patronymic, Gender_Feminine },
	{ _T("ов,цын,ин"), FioPart_FamilyName, Gender_Masculine },
	{ _T("ова,цына,ина"), FioPart_FamilyName, Gender_Feminine },
	{ _T("ой,ый"), FioPart_FamilyName, Gender_Masculine },
	{ _T("ая"), FioPart_FamilyName, Gender_Feminine },
	{ _T("ых"), FioPart_FamilyName, Gender_Unisex }
};

static std::map<std::tstring, RodChislo> abbreviations{
	{ _T("ооо"), RodChislo::SingNeut },
	{ _T("ао"), RodChislo::SingNeut },
	{ _T("пао"), RodChislo::SingNeut },
	{ _T("оао"), RodChislo::SingNeut },
	{ _T("зао"), RodChislo::SingNeut },
	{ _T("нтп"), RodChislo::SingNeut },
	{ _T("нии"), RodChislo::SingMasc },
	{ _T("муп"), RodChislo::SingNeut },
	{ _T("гуп"), RodChislo::SingNeut },
	{ _T("фгуп"), RodChislo::SingNeut },
	{ _T("мдоу"), RodChislo::SingNeut }, // Муниципальное дошкольное образовательное учреждение
};

// defined in dict.cpp
extern char dict[];
extern int dict_size;

// defined in fioDict.cpp
extern char fioDict[];
extern int fioDict_size;

// defined in specialDict.cpp
extern char specialDict[];
extern int specialDict_size;

// defined in specialAdjDict.cpp
extern char specialAdjDict[];
extern int specialAdjDict_size;

class Global 
{
public:
	static size_t GetInt32(std::stringstream & file)
	{
		size_t entryCount = static_cast<unsigned char>(file.get());
		entryCount += static_cast<size_t>(static_cast<unsigned char>(file.get())) << 8;
		entryCount += static_cast<size_t>(static_cast<unsigned char>(file.get())) << 16;
		entryCount += static_cast<size_t>(static_cast<unsigned char>(file.get())) << 24;
		return entryCount;
	}
};

static const _Indirect Indirect;