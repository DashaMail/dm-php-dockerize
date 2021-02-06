#pragma once
#include "Endings.h"

class SingularOrPluralGramInfo
{
	const Endings * endings;
	bool fleetingVowel;
	bool endingStressed;

public:

	SingularOrPluralGramInfo(); // Для создания массива в GramInfoFactory

	SingularOrPluralGramInfo(
		const Endings* endings,
		bool fleetingVowel,
		bool endingStressed
	);

	const Endings* GetEndings() const;
	bool EndingStressed() const;
	bool FleetingVowel() const;
};
