// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "Phrase.h"

Phrase::Phrase(
	const std::list<GC::Ptr<const IPhrasePart>>& parts,
	RodChislo rodChislo,
	bool isAnimate,
	bool prepositionNA,
	const Phrase* pluralPhrase)
	: parts(parts),
	rodChislo(rodChislo),
	isAnimate(isAnimate),
	prepositionNA(prepositionNA),
	pluralPhrase(pluralPhrase),
	fio(nullptr)
{
}

Phrase::Phrase(
	const std::list<GC::Ptr<const IPhrasePart>>& parts,
	RodChislo rodChislo,
	bool isAnimate,
	bool prepositionNA,
	const std::string& fioCombination,
	const std::list<MultiPartDecoratedWord>& components,
	const Phrase* pluralPhrase)
	: parts(parts),
	rodChislo(rodChislo),
	isAnimate(isAnimate),
	prepositionNA(prepositionNA),
	pluralPhrase(pluralPhrase)
{
	if (!fioCombination.empty())
	{
		this->fio = new FIO(components, fioCombination);
	}
	else
	{
		this->fio = nullptr;
	}
}

Phrase::Phrase(
	const Phrase* phrase1,
	const Phrase* phrase2)
	: parts(Concat(phrase1->parts, phrase2->parts)),
	rodChislo(phrase1->rodChislo),
	isAnimate(phrase1->isAnimate),
	prepositionNA(phrase1->prepositionNA),
	pluralPhrase(phrase1->pluralPhrase && phrase2->pluralPhrase
		? new Phrase(phrase1->pluralPhrase, phrase2->pluralPhrase)
		: 0),
	fio(nullptr)
{
}

Phrase::Phrase(
	const std::unique_ptr<Phrase>& phrase1,
	const std::unique_ptr<Phrase>& phrase2)
	: parts(Concat(phrase1->parts, phrase2->parts)),
	rodChislo(phrase1->rodChislo),
	isAnimate(phrase1->isAnimate),
	prepositionNA(phrase1->prepositionNA),
	pluralPhrase(phrase1->pluralPhrase && phrase2->pluralPhrase
		? new Phrase(phrase1->pluralPhrase, phrase2->pluralPhrase)
		: 0),
	fio(nullptr)
{
}

std::list<GC::Ptr<const IPhrasePart>> Phrase::Concat(
	const std::list<GC::Ptr<const IPhrasePart>> list1,
	const std::list<GC::Ptr<const IPhrasePart>> list2)
{
	std::list<GC::Ptr<const IPhrasePart>> result;
	result.insert(result.end(), list1.begin(), list1.end());
	result.insert(result.end(), list2.begin(), list2.end());
	return result;
}

std::tstring Phrase::getImen() const
{
	return GetForm(Padeg_I);
}

std::tstring Phrase::getRod() const
{
	return GetForm(Padeg_R);
}

std::tstring Phrase::getDat() const
{
	return GetForm(Padeg_D);
}

std::tstring Phrase::getVin() const
{
	return GetForm(Padeg_V);
}

std::tstring Phrase::getTvor() const
{
	return GetForm(Padeg_T);
}

std::tstring Phrase::getPred() const
{
	return GetForm(Padeg_P);
}

std::tstring Phrase::getPredO() const
{
	return GetPrepositional(&Phrase::ChoosePrepositionO, Padeg_P);
}

std::tstring Phrase::getMest() const
{
	return GetPrepositional(&Phrase::ChoosePrepositionVorNA, Padeg_M);
}

RodChislo Phrase::getRodChislo() const
{
	return rodChislo;
}

bool Phrase::IsAnimate() const
{
	return isAnimate;
}

const Phrase* Phrase::Plural() const
{
	return pluralPhrase;
}

const FIO* Phrase::Fio() const
{
	return fio;
}

std::tstring Phrase::GetPrepositional(
	ChoosePrepositionDelegate choosePreposition, Padeg padeg) const
{
	return AddPreposition(GetForm(padeg), choosePreposition);
}

std::tstring Phrase::AddPreposition(
	const std::tstring& form,
	ChoosePrepositionDelegate choosePreposition) const
{
	if (form.empty()) return form;

	Tokenizer t(form, Tokenizer::Russian);

	// Взять 2 первых слова.
	std::tstring firstSeparator = t.GetNextSeparator();
	std::tstring w0 = t.GetNextToken();
	t.SkipNextSeparator();
	std::tstring w1 = t.GetNextToken();
	Caps::ToLower(w0);
	Caps::ToLower(w1);

	// "ни о ком, ни о чём, кое о ком, кое о чём"
	if ((w0 == _T("ни") || w0 == _T("кое")) && w1 == _T("о")) return form;

	// добавить предлог о/об/обо перед первым словом
	size_t i = firstSeparator.length();

	// Предлог должен стоять перед кавычкой: о "Варяге", но (о Варяге).
	if (Common::EndsWithOneOf(firstSeparator, _T("\",',«,“,”"))) --i;

	std::tstring preposition = (this->*choosePreposition)(w0);

	// Если вся фраза большими буквами, то и предлог сделать тоже.
	if (Caps::IsUpper(form.begin(), form.end())) Caps::ToUpper(preposition);

	return form.substr(0, i) + preposition + _T(' ') + form.substr(i);
}

const TCHAR* Phrase::ChoosePrepositionO(const std::tstring& firstWord) const
{
	static const TCHAR* exceptions[] =
	{
		_T("мне"),
		_T("всём"),
		_T("всем") // "всём" без точек над Ё
	};

	for (size_t i = 0; i < sizeof(exceptions) / sizeof(*exceptions); ++i)
	{
		if (firstWord == exceptions[i]) return _T("обо");
	}

#ifdef _UNICODE
	std::tstring vowels = _T("аоуэиыaeiouαεοω");
#else
	std::tstring vowels = _T("аоуэиыaeio");
#endif

	// буквы, названия которых начинаются с гласной, e.g. "эм", "икс"
	std::tstring consonants = _T("лмнсфfhlmnrsx");

	bool containsVowel = firstWord.find_first_of(vowels + _T("яёею")) != std::tstring::npos;

	return
		(containsVowel ? vowels : consonants).find_first_of(firstWord[0])
		==
		std::tstring::npos ? _T("о") : _T("об");
}

const TCHAR* Phrase::ChoosePrepositionVorNA(const std::tstring& firstWord) const
{
	return prepositionNA ? _T("на") : ChoosePrepositionVorVO(firstWord);
}

const TCHAR* Phrase::ChoosePrepositionVorVO(const std::tstring& firstWord) const
{
	static const TCHAR* exceptions[] =
	{
		_T("дворе"),
		_T("дворах"), // но: в дверях
		_T("рту"),
		_T("ртах"),
		_T("сне"),
		_T("снах"),
		_T("ржи"), // но: в ржавчине
		_T("ржах"),
		_T("мне"), // но: в мнении, в мнимой величине
	};

	for (size_t i = 0; i < sizeof(exceptions) / sizeof(*exceptions); ++i)
	{
		if (firstWord == exceptions[i]) return _T("во");
	}

	if (firstWord.length() > 3
		&& (firstWord[0] == _T('в')
			|| firstWord[0] == _T('ф'))
		&& !Common::IsVowel(firstWord[1]))
	{
		// во времени, во власти, во Владимире, во Франции, во Флоренции
		return _T("во");
	}

	// во Львове, во Льгове

	// TODO
	//return (StartsWithOneOf (firstWord, "ль,мц")) ? "во" : "в";

	return _T("в");
}

std::tstring Phrase::GetForm(Padeg padeg) const
{
	std::tstring s = _T("");

	for (auto part = parts.begin(); part != parts.end(); ++part)
	{
		// Вижу животное, дитятко, привидение (сред.род) - склоняются как неодушевлённые.
		// Вижу животных, дитяток, привидений (мн.ч) - как одушевлённые.
		// Вижу подмастерья (м.р.) - как одушевлённое.
		GC::Ptr<const IPhrasePart> p = *part;
		std::tstring form = p->GetFormByCase(padeg, isAnimate && rodChislo != SingNeut);
		if (form.empty()) return form;
		s += form;
	}

	return s;
}

std::tstring Phrase::PaucalForm() const
{
	std::tstring s;

	std::list<GC::Ptr<const IPhrasePart>>::const_iterator singularIter = parts.begin();
	std::list<GC::Ptr<const IPhrasePart>>::const_iterator pluralIter;
	if (pluralPhrase) pluralIter = pluralPhrase->parts.begin();

	for (; singularIter != parts.end(); ++singularIter)
	{
		GC::Ptr<const IPhrasePart> p =
			(*singularIter)->PaulcalIsSingular()
			? *singularIter : pluralPhrase
			? *pluralIter : GC::Ptr<const IPhrasePart>(nullptr);
		if (!p) return _T("");

		std::tstring form = p->GetFormByCase(Padeg_R, isAnimate);
		if (form.empty()) return form;

		s += form;

		if (pluralPhrase) ++pluralIter;
	}

	return s;
}
