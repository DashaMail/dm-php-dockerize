#pragma once
#include "Info.h"
#include "SingularOrPluralGramInfo.h"

// Грамматическая информация о слове в именительном падеже.
class GramInfo : public Info
{
private:
	SingularOrPluralGramInfo singular;
	SingularOrPluralGramInfo plural;

public:
	GramInfo(); // Для создания массива в Dictionary.

	GramInfo(RodChislo rodChislo, bool IsNoun,
	         const ::Endings* endings, bool endingIsStressed, bool hasVolatileVowel,
	         const ::Endings* endingsPlural, bool endingIsStressedPlural,
	         bool IsAnimate, bool Alteration, bool Na, bool yo);

	const SingularOrPluralGramInfo* GetSingular() const;
	const SingularOrPluralGramInfo* GetPlural() const;

	const Endings* GetEndings() const;
	const Endings* GetEndingsPlural() const;
	RodChislo rodChislo;
	bool IsNoun;
	bool EndingIsStressed() const; // ending ударное
	bool EndingIsStressedPlural() const; // ending ударное во множественном
	bool IsAnimate; // одушевлённое
	bool Alteration; // чередование
	bool Na; // местный с предлогом НА
	bool Yo;

	virtual const GramInfo* Nominative() const override;
};
