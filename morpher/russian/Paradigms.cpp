// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "Paradigms.h"

Paradigms::Row Paradigms::Uninflected = { { _T(""),     _T(""),    _T(""),  _T(""),   _T(""),    _T("") }, false, EndingsType_Uninflected };
Paradigms::Row Paradigms::PronomialSingularFeminine = { { _T("а"),  _T("ой"),  _T("ой"), _T("у"),  _T("ой"), _T("ой") }, false, EndingsType_Pronomial };
Paradigms::Row Paradigms::PronomialSingularMasculine = { { _T(""),  _T("ого"), _T("ому"),  _T(""),  _T("ым"), _T("ом") }, true,  EndingsType_Pronomial };
Paradigms::Row Paradigms::PronomialSingularMasculineOV = { { _T(""),    _T("а"),   _T("у"),  _T(""),  _T("ым"), _T("ом") }, true,  EndingsType_Pronomial };
Paradigms::Row Paradigms::PronomialSingularNeuter = { { _T("о"), _T("ого"), _T("ому"), _T("о"),  _T("ым"), _T("ом") }, true,  EndingsType_Pronomial };
Paradigms::Row Paradigms::PronomialSingularNeuterOV = { { _T("о"),   _T("а"),   _T("у"), _T("о"),  _T("ым"), _T("ом") }, true,  EndingsType_Pronomial };
Paradigms::Row Paradigms::PronomialPlural = { { _T("ы"),  _T("ых"),  _T("ым"), _T("ы"), _T("ыми"), _T("ых") }, true,  EndingsType_Pronomial };
Paradigms::Row Paradigms::AdjectivalSingularFeminine = { { _T("ая"), _T("ой"),  _T("ой"), _T("ую"), _T("ой"), _T("ой") }, false, EndingsType_Adjectival };
Paradigms::Row Paradigms::AdjectivalSingularMasculineEndingStressed = { { _T("ой"),_T("ого"), _T("ому"), _T("ой"), _T("ым"), _T("ом") }, true,  EndingsType_Adjectival };
Paradigms::Row Paradigms::AdjectivalSingularMasculineStemStressed = { { _T("ый"),_T("ого"), _T("ому"), _T("ый"), _T("ым"), _T("ом") }, true,  EndingsType_Adjectival };
Paradigms::Row Paradigms::AdjectivalSingularNeuter = { { _T("ое"),_T("ого"), _T("ому"), _T("ое"), _T("ым"), _T("ом") }, true,  EndingsType_Adjectival };
Paradigms::Row Paradigms::AdjectivalPlural = { { _T("ые"), _T("ых"),  _T("ым"), _T("ые"),_T("ыми"), _T("ых") }, true,  EndingsType_Adjectival };
Paradigms::Row Paradigms::Noun1stDecl = { { _T("а"),   _T("ы"),   _T("е"), _T("у"),  _T("ой"),  _T("е") }, false, EndingsType_NounLike };
Paradigms::Row Paradigms::Noun2ndDeclMasculine = { { _T(""),   _T("а"),   _T("у"),  _T(""),  _T("ом"),  _T("е") }, true,  EndingsType_NounLike };
Paradigms::Row Paradigms::Noun2ndDeclMasculineU = { { _T(""),   _T("а"),   _T("у"),  _T(""),  _T("ом"),  _T("е"),  _T("у") }, true, EndingsType_NounLike };
Paradigms::Row Paradigms::Noun2ndDeclNeuter = { { _T("о"),   _T("а"),   _T("у"), _T("о"),  _T("ом"),  _T("е") }, true,  EndingsType_NounLike };
Paradigms::Row Paradigms::Noun3rdDecl = { { _T(""),   _T("и"),   _T("и"),  _T(""),  _T("ью"),  _T("и") }, false, EndingsType_NounLike };
Paradigms::Row Paradigms::MasculineFamilyName = { { _T(""),    _T("а"),   _T("у"),  _T(""),  _T("ым"),  _T("е") }, true, EndingsType_Pronomial };

Paradigms::Row * Paradigms::table[18] = {
	&Paradigms::Uninflected,
	&Paradigms::PronomialSingularFeminine,
	&Paradigms::PronomialSingularMasculine,
	&Paradigms::PronomialSingularMasculineOV,
	&Paradigms::PronomialSingularNeuter,
	&Paradigms::PronomialSingularNeuterOV,
	&Paradigms::PronomialPlural,
	&Paradigms::AdjectivalSingularFeminine,
	&Paradigms::AdjectivalSingularMasculineEndingStressed,
	&Paradigms::AdjectivalSingularMasculineStemStressed,
	&Paradigms::AdjectivalSingularNeuter,
	&Paradigms::AdjectivalPlural,
	&Paradigms::Noun1stDecl,
	&Paradigms::Noun2ndDeclMasculine,
	&Paradigms::Noun2ndDeclMasculineU,
	&Paradigms::Noun2ndDeclNeuter,
	&Paradigms::Noun3rdDecl,
	&Paradigms::MasculineFamilyName,

	// СущМнож - отдельный класс SubstPlural
};
