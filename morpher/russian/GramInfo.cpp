// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "GramInfo.h"

GramInfo::GramInfo()
{
}

GramInfo::GramInfo(
	RodChislo rodChislo,
	bool IsNoun,
	const ::Endings* endings,
	bool endingIsStressed,
	bool hasVolatileVowel,
	const ::Endings* endingsPlural,
	bool endingIsStressedPlural,
	bool IsAnimate,
	bool Alteration,
	bool Na,
	bool yo)
	: singular(endings, hasVolatileVowel, endingIsStressed)
	, plural(endingsPlural, hasVolatileVowel, endingIsStressedPlural)
	, rodChislo(rodChislo)
	, IsNoun(IsNoun)
	, IsAnimate(IsAnimate)
	, Alteration(Alteration)
	, Na(Na)
	, Yo(yo)
{
}

const SingularOrPluralGramInfo* GramInfo::GetSingular() const
{
	return &singular;
}

const SingularOrPluralGramInfo* GramInfo::GetPlural() const
{
	return &plural;
}

const Endings* GramInfo::GetEndings() const
{
	return singular.GetEndings();
}

const Endings* GramInfo::GetEndingsPlural() const
{
	return plural.GetEndings();
}

bool GramInfo::EndingIsStressed() const
{
	return singular.EndingStressed();
}

bool GramInfo::EndingIsStressedPlural() const
{
	return plural.EndingStressed();
}

const GramInfo* GramInfo::Nominative() const
{
	return this;
}
