// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "SingularOrPluralGramInfo.h"

SingularOrPluralGramInfo::SingularOrPluralGramInfo()
{
}

SingularOrPluralGramInfo::SingularOrPluralGramInfo(
	const Endings* endings,
	bool fleetingVowel,
	bool endingStressed)
	: endings(endings)
	, fleetingVowel(fleetingVowel)
	, endingStressed(endingStressed)
{
}

const Endings* SingularOrPluralGramInfo::GetEndings() const
{
	return endings;
}

bool SingularOrPluralGramInfo::EndingStressed() const
{
	return endingStressed;
}

bool SingularOrPluralGramInfo::FleetingVowel() const
{
	return fleetingVowel;
}
