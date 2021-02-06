#pragma once
#include <list>
#include <memory>
#include "../gc.h"
#include "../tstring.h"
#include "IPhrasePart.h"
#include "Fio.h"

class Phrase // C#: Склоняемое
{
	const std::list <GC::Ptr <const IPhrasePart> > parts;
	RodChislo rodChislo;
	bool isAnimate;
	bool prepositionNA;
	const Phrase * pluralPhrase;
	const FIO * fio;

public:

	Phrase(const std::list<GC::Ptr<const IPhrasePart>>& parts,
	       RodChislo rodChislo,
	       bool isAnimate,
	       bool prepositionNA,
	       const Phrase* pluralPhrase);

	Phrase(const std::list<GC::Ptr<const IPhrasePart>>& parts,
	       RodChislo rodChislo,
	       bool isAnimate,
	       bool prepositionNA,
	       const std::string& fioCombination,
	       const std::list<MultiPartDecoratedWord>& components,
	       const Phrase* pluralPhrase);

	Phrase(const Phrase* phrase1, const Phrase* phrase2);

	Phrase(const std::unique_ptr<Phrase>& phrase1, const std::unique_ptr<Phrase>& phrase2);

	static std::list<GC::Ptr<const IPhrasePart>> Concat(
		const std::list<GC::Ptr<const IPhrasePart>> list1,
		const std::list<GC::Ptr<const IPhrasePart>> list2);

	std::tstring getImen() const;
	std::tstring getRod() const;
	std::tstring getDat() const;
	std::tstring getVin() const;
	std::tstring getTvor() const;
	std::tstring getPred() const;
	std::tstring getPredO() const;
	std::tstring getMest() const;

	RodChislo getRodChislo() const;

	bool IsAnimate() const;

	const Phrase* Plural() const;

	const FIO* Fio() const;

private:

	/// <param name="word">слово перед предлогом</param>
	typedef const TCHAR * (Phrase::*ChoosePrepositionDelegate)
						  (const std::tstring & word) const;

	std::tstring GetPrepositional(
		ChoosePrepositionDelegate choosePreposition,
		Padeg padeg) const;

	std::tstring AddPreposition(
		const std::tstring& form,
		ChoosePrepositionDelegate choosePreposition) const;

	const TCHAR* ChoosePrepositionO(const std::tstring& firstWord) const;

	const TCHAR* ChoosePrepositionVorNA(const std::tstring& firstWord) const;

	const TCHAR* ChoosePrepositionVorVO(const std::tstring& firstWord) const;

public:
	std::tstring GetForm(Padeg padeg) const;

	std::tstring PaucalForm() const;
};