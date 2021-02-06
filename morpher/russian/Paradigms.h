#pragma once

#include "Global.h"
// конкретные парадигмы (данные для реализации интерфейса Endings)
namespace Paradigms
{
	struct Row
	{
		const TCHAR * endings[7];
		bool accAnimEqualsGen;
		EndingsType endingsType;
	};

	extern Row Uninflected;
	extern Row PronomialSingularFeminine;
	extern Row PronomialSingularMasculine;
	extern Row PronomialSingularMasculineOV;
	extern Row PronomialSingularNeuter;
	extern Row PronomialSingularNeuterOV;
	extern Row PronomialPlural;
	extern Row AdjectivalSingularFeminine;
	extern Row AdjectivalSingularMasculineEndingStressed;
	extern Row AdjectivalSingularMasculineStemStressed;
	extern Row AdjectivalSingularNeuter;
	extern Row AdjectivalPlural;
	extern Row Noun1stDecl;
	extern Row Noun2ndDeclMasculine;
	extern Row Noun2ndDeclMasculineU;
	extern Row Noun2ndDeclNeuter;
	extern Row Noun3rdDecl;
	extern Row MasculineFamilyName;

	extern Row * table[18];
};