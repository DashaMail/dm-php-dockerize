// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iterator>
#include <stdexcept>
#include "morpher.h"
#include "TokenizedString.h"

const std::tstring Common::vowels = _T("аяоёиыуюэе");

void RussianDeclensor::Tokenize(
	const std::tstring& s,
	std::list<DecoratedWord>& decoratedWords,
	std::list<std::tstring>& separators)
{
	Tokenizer t = Tokenizer(s, Tokenizer::Russian);

	for (;;)
	{
		separators.push_back(t.GetNextSeparator());

		std::tstring word = t.GetNextToken();

		if (word.empty()) break;

		Tokenizer::ChangeLatinToCyrillic(&word);

		DecoratedWord decoratedWord = DecoratedWord::Get(word);

		decoratedWords.push_back(decoratedWord);
	}
}

void RussianDeclensor::RemoveNonRussian(
	std::list<DecoratedWord>& decoratedWords,
	std::list<std::tstring>& separators)
{
	std::list<DecoratedWord>::iterator word = decoratedWords.begin();
	std::list<std::tstring>::iterator separator = separators.begin();

	while (word != decoratedWords.end())
	{
		if (Tokenizer::IsRussian(word->Lemma()))
		{
			++word;
			++separator;
		}
		else
		{
			std::list<DecoratedWord>::iterator wordToRemove = word;
			++word;
			std::tstring wordToRemoveLemma = wordToRemove->DecoratedLemma();
			decoratedWords.erase(wordToRemove);

			std::list<std::tstring>::iterator nextSep = separator;
			++nextSep;
			*separator += wordToRemoveLemma + *nextSep;
			separators.erase(nextSep);
		}
	}
}

bool RussianDeclensor::IsHyphen(const std::tstring& s)
{
	return s == _T("-") || s == _T("–");
}

void RussianDeclensor::JoinHypenatedWords(
	std::list<DecoratedWord>& decoratedWords,
	std::list<std::tstring>& separators,
	std::list<MultiPartDecoratedWord>& result)
{
	std::list<DecoratedWord>::iterator wordNode = decoratedWords.begin();
	std::list<std::tstring>::iterator separatorNode = separators.begin();

	for (; wordNode != decoratedWords.end(); ++wordNode)
	{
		++separatorNode;

		std::vector<DecoratedWord> parts;

		parts.push_back(*wordNode);

		while (++wordNode != decoratedWords.end() && IsHyphen(*separatorNode))
		{
			parts.push_back(*wordNode);
			++separatorNode;
		}

		--wordNode;

		result.push_back(MultiPartDecoratedWord(parts));
	}
}

template <class T>
void RussianDeclensor::Move(std::list<T>& to, std::list<T>& from)
{
	assert(from.size() > 0);

	to.splice(to.end(), from, from.begin(), ++from.begin());
}

void RussianDeclensor::Move(
	std::list<std::tstring>& currentDelimList,
	std::list<std::tstring>& separators,
	const std::list<MultiPartDecoratedWord>& currentWordList)
{
	for (size_t i = 0; i < currentWordList.rbegin()->PartCount(); ++i)
	{
		Move(currentDelimList, separators);
	}
}

std::tstring RussianDeclensor::Trim(const std::tstring& s)
{
	const TCHAR* ws = _T(" .\t\n\r");

	size_t first = s.find_first_not_of(ws);
	if (first == std::tstring::npos) return _T("");

	size_t last = s.find_last_not_of(ws);
	if (last == std::tstring::npos) return _T("");

	return s.substr(first, last - first + 1);
}

bool RussianDeclensor::IsDash(const std::tstring& s)
{
	std::tstring p = Trim(s);

	if (p == _T("-")) return true;
	if (p == _T("--")) return true;
	if (p == _T("–")) return true;
	if (p == _T("—")) return true;

	return false;
}

bool RussianDeclensor::IsInitial(const std::tstring& s)
{
	return s.length() == 1 || !Common::ContainsVowel(s);
}

bool RussianDeclensor::IsInitial(const MultiPartDecoratedWord& multiPartDecoratedWord)
{
	for (auto part = multiPartDecoratedWord.Parts().begin();
		part != multiPartDecoratedWord.Parts().end(); ++part)
	{
		if (!IsInitial(part->Lemma())) return false;
	}

	return true;
}

void RussianDeclensor::HandleInitials(
	std::list<MultiPartDecoratedWord>& components,
	std::list<std::tstring>& separators)
{
	std::list<MultiPartDecoratedWord>::iterator component = components.begin();
	std::list<std::tstring>::iterator separator = separators.begin();

	while (component != components.end())
	{
		if (IsInitial(*component))
		{
			std::tstring s = *separator;

			for (auto word = component->Parts().begin();
				word != component->Parts().end(); ++word)
			{
				std::list<std::tstring>::iterator previous = separator;
				++separator;
				separators.erase(previous);
				s += word->DecoratedLemma() + *separator;
			}
			*separator = s;
			std::list<MultiPartDecoratedWord>::iterator componentToRemove = component;
			++component;
			components.erase(componentToRemove);
		}
		else
		{
			++component;
			++separator;
		}
	}
}

bool RussianDeclensor::NotFoundInDictionaryOrInfectionIsAmbiguous(
	const std::tstring& component) const
{
	bool foundInDictionary = fioDict.Contains(component);
	Gender genderAsPerDictionary = dictionary.GetGender(component);
	bool endsInConsonant = Common::IsConsonant(*component.rbegin());

	bool result = !foundInDictionary
		|| (genderAsPerDictionary
			== Gender_Feminine
			&& endsInConsonant);

	return result;
}

const GramInfo* RussianDeclensor::Uninflected(RodChislo rodChislo, bool isNoun) const
{
	return dictionary.Uninflected(rodChislo, isNoun);
}

std::string RussianDeclensor::ChooseCombination(
	const std::list<MultiPartDecoratedWord>& components
	, RodChislo* rodChisloResult,
	Attributes atts) const
{
	// HACK для "батоно Пак Гульфияр..."
	if (components.size() > 3)
	{
		*rodChisloResult = SingMasc;
		return std::string(components.size(), 'I');
	}

	int both = Attribute_Masc | Attribute_Fem;

	// Если ни одного рода не указано, это равносильно тому, что указаны оба,
	// т.е. род определяется автоматически.
	// В .NET версии это не так - указание обоих приводит к исключению.
	if ((atts & both) == 0) atts = static_cast<Attributes>(atts | both);

	if (components.size() == 2)
	{
		const char* combinations[] = { "FI", "IF" };

		std::vector<MultiPartDecoratedWord> sost(components.begin(), components.end());

		for (int i = 0; i < 2; ++i)
		{
			const std::tstring& familyName = sost[i].LastPart();
			const std::tstring& givenName = sost[1 - i].LastPart();

			// Патч для "Герцен Элла" - обе составляющие есть в словаре как имена,
			// и только одно есть как familyName (Герцен).
			if (fioDict.Contains(familyName, FioPart_GivenName)
				&& fioDict.Contains(givenName, FioPart_GivenName)
				&& fioDict.Contains(familyName, FioPart_FamilyName)
				&& !fioDict.Contains(givenName, FioPart_FamilyName))
			{
				switch (dictionary.GetGender(givenName))
				{
				case Gender_Masculine:
					*rodChisloResult = SingMasc;
					break;
				case Gender_Feminine:
					*rodChisloResult = SingFem;
					break;
				case Gender_Unisex:
					continue;
				default:
					throw std::exception(); // "Неправильный Gender."
				}
				return combinations[i];
			}

			// патч для "Асадов Наргиз", "Иванов Женя"
			if (NotFoundInDictionaryOrInfectionIsAmbiguous(familyName) &&
				NotFoundInDictionaryOrInfectionIsAmbiguous(givenName))
			{
				if (Common::HasEndings(familyName, _T("ов,ын,ой,ый,ович")))
				{
					*rodChisloResult = SingMasc;
					return combinations[i];
				}
				if (Common::HasEndings(familyName, _T("ова,ына,ая,овна")))
				{
					*rodChisloResult = SingFem;
					return combinations[i];
				}
				if (Common::EndsWithOneOf(familyName, _T("дзе,ян,енко")))
				{
					*rodChisloResult
						= ConvertGenderToRodChislo(dictionary.GetGender(givenName));
					return combinations[i];
				}
			}
		}
	}

	{
		std::string bestCombination = "";
		RodChislo bestRodChislo = SingMasc;

		int bestScore = -1;

		std::string combinations[] = { "FIO","IOF","IF","FI","IO","F","I","O" };

		for (size_t k = 0; k < sizeof(combinations) / sizeof(*combinations); ++k)
		{
			const std::string& combination = combinations[k];

			if (components.size() != combination.length()) continue;

			RodChislo rodChislo = SingMasc;

			for (int j = 0; j < 2; ++j, rodChislo = Opposite(rodChislo))
			{
				Attributes rodattr = rodChislo == SingMasc ? Attribute_Masc : Attribute_Fem;

				if ((atts & rodattr) == 0)
					continue;

				int score = 0;

				unsigned i = 0;

				for (auto component = components.begin();
					i < components.size();
					++component, ++i)
				{
					FioPart fioPart = GetFioPart(combination[i]);

					const std::tstring& lastPart = component->LastPart();

					if (fioDict.Contains(component->Lemma(), fioPart))
					{
						if (GenderMatchesRodChislo(lastPart, rodChislo, fioPart))
						{
							// Именам дается приоритет по сравнению с фамилиями.
							// "Айрих Любовь", "Галина Рерих" и т.п. - выигрывает имя.
							score += fioPart == FioPart_FamilyName ? 2 : 3;
						}
						if ((atts & both)
							== both
							&& GenderMatchesRodChislo(
								lastPart,
								Opposite(rodChislo),
								fioPart))
						{
							score -= 1;
						}
					}
					else
					{
						FioPartAndGender partAndGender = GetfioPartAndGender(*component);

						if (partAndGender.fioPart
							== fioPart
							&& Matches(partAndGender.gender, rodChislo))
						{
							score += 1;
						}
					}
				}

				if (bestScore < score)
				{
					bestScore = score;
					bestCombination = combination;
					bestRodChislo = rodChislo;
				}
			}
		}

		*rodChisloResult = bestRodChislo;
		return bestCombination;
	}
}

RodChislo RussianDeclensor::ConvertGenderToRodChislo(Gender rodChislo)
{
	switch (rodChislo)
	{
	case Gender_Masculine: return SingMasc;
	case Gender_Feminine: return SingFem;
	case Gender_Unisex: return SingMasc;
	}

	throw std::exception(); // "Неправильный Gender."
}

bool RussianDeclensor::Matches(Gender gender, RodChislo rodChislo)
{
	if (gender == Gender_Unisex) return true;

	if (gender == Gender_Masculine && rodChislo == SingMasc) return true;

	if (gender == Gender_Feminine && rodChislo == SingFem) return true;

	return false;
}

bool RussianDeclensor::GenderMatchesRodChislo(
	const std::tstring& lemma,
	RodChislo rodChislo,
	FioPart fioPart) const
{
	if (lemma == _T("лука")) return rodChislo == SingMasc;

	return FindInCommonDictionary(lemma, rodChislo, fioPart) != nullptr;
}

RussianDeclensor::FioPartAndGender::FioPartAndGender(
	FioPart fioPart,
	Gender gender) : fioPart(fioPart), gender(gender)
{
}

RussianDeclensor::FioPartAndGender RussianDeclensor::GetFioPartAndGenderNullable(
	const std::tstring& word)
{
	for (size_t i = 0; i < sizeof(suffixes) / sizeof(*suffixes); ++i)
	{
		if (Common::HasEndings(word, suffixes[i].Morphs))
		{
			return FioPartAndGender(suffixes[i].fioPart, suffixes[i].gender);
		}
	}

	return FioPartAndGender(static_cast<FioPart>(-1), Gender_Feminine);
}

RussianDeclensor::FioPartAndGender RussianDeclensor::GetfioPartAndGender(
	const MultiPartDecoratedWord& word) const
{
	for (auto part = word.Parts().begin(); part != word.Parts().end(); ++part)
	{
		FioPartAndGender fioPartAndGender = GetFioPartAndGenderNullable(part->Lemma());

		if (fioPartAndGender.fioPart != static_cast<FioPart>(-1)) return fioPartAndGender;
	}

	return FioPartAndGender(FioPart_GivenName, dictionary.GetGender(word.LastPart()));
}

const SingularOrPluralGramInfo* RussianDeclensor::FindInCommonDictionary(
	const std::tstring& lemma,
	RodChislo rodChislo,
	FioPart fioPart) const
{
	Dictionary::ResultSet gramInfos;
	dictionary.Find(lemma, true, gramInfos);

	if (gramInfos.empty()) return nullptr;

	// фамилии, совпадающие с нарицательными 3-го склонения, не склоняются (Мышь, Грусть)
	if (fioPart == FioPart_FamilyName
		&& gramInfos.size() == 1
		&& (*(*gramInfos.begin())->GetEndings())[Padeg_T] == _T("ью"))
	{
		return Uninflected(rodChislo, true)->GetSingular();
	}

	const GramInfo* gramInfo = nullptr;

	for (auto gi = gramInfos.begin(); gi != gramInfos.end(); ++gi)
	{
		if ((*gi)->rodChislo == rodChislo)
		{
			gramInfo = *gi;
			break;
		}
	}

	if (gramInfo)
	{
		if (gramInfo->GetEndings()->GetEndingsType() == EndingsType_Pronomial)
		{
			// Местоимённые собственные - это фамилии (Иванов), 
			// в т.ч. польские типа Стефаньска, Покорны.
			if (Common::EndsWithOneOf(lemma, _T("и,ы,а"))
				|| Common::HasPronomialFamilyNameSuffix(lemma))
			{
				return gramInfoFactory().GetGramInfo(rodChislo == SingMasc ?
					&Paradigms::MasculineFamilyName
					: &Paradigms::PronomialSingularFeminine, gramInfo->EndingIsStressed());
			}
			else
			{
				// Просто совпало с местоимённым нарицательным.  Делаем его субстантивным.
				// Бабий, Чебан
				return gramInfoFactory().GetGramInfo(
					&Paradigms::Noun2ndDeclMasculine,
					false);
			}
		}
	}

	return gramInfo == nullptr ? NULL : gramInfo->GetSingular();
}

const GramInfoFactory& RussianDeclensor::gramInfoFactory() const
{
	return dictionary.getGramInfoFactory();
}

RodChislo RussianDeclensor::Opposite(RodChislo rodChislo)
{
	return (RodChislo)((int)SingMasc + (int)SingFem - (int)rodChislo);
}

FioPart RussianDeclensor::GetFioPart(char c)
{
	switch (c)
	{
	case 'F': return FioPart_FamilyName;
	case 'I': return FioPart_GivenName;
	case 'O': return FioPart_Patronymic;
	}

	throw std::exception();
}

RussianDeclensor::ParsedPartFIO::ParsedPartFIO(
	const MultiPartDecoratedWord& multiPartDecoratedWord,
	const std::vector<const SingularOrPluralGramInfo*>& gramInfo)
	: multiPartDecoratedWord(multiPartDecoratedWord)
	, gramInfo(gramInfo)
{
}

bool RussianDeclensor::ParsedPartFIO::AppendTo(
	PhraseBuilder& phraseBuilder,
	std::list<std::tstring>::const_iterator& separator,
	bool plural) const
{
	if (plural) return false;

	for (size_t part = 0; part < multiPartDecoratedWord.PartCount(); ++part)
	{
		if (part != 0)
		{
			phraseBuilder.Add(*separator++);
		}

		const ParameterlessParadigm& se =
			*static_cast<const ParameterlessParadigm *>(gramInfo[part]->GetEndings());
		std::tstring t = se[Padeg_T];
		std::tstring p = se[Padeg_P];

		phraseBuilder.Add(new SingleWordWithGramInfo(
			gramInfo[part], multiPartDecoratedWord.Parts()[part]));
	}

	return true;
}

GC::Ptr<IParsedPart> RussianDeclensor::ParsedPartFIO::GetPlural() const
{
	return nullptr;
}

void RussianDeclensor::AnalyseFio(
	std::list<MultiPartDecoratedWord>& components,
	std::list<std::tstring>& separators,
	Attributes attributes,
	RodChislo* rodChislo,
	bool* isAnimate,
	bool* prepositionNA,
	std::string& fioCombination,
	std::list<GC::Ptr<IParsedPart>>& result) const
{
	*prepositionNA = false;

	*isAnimate = true;

	HandleInitials(components, separators);

	std::string combination = ChooseCombination(components, rodChislo, attributes);
	fioCombination = combination;

	std::list<MultiPartDecoratedWord>::const_iterator component = components.begin();

	for (size_t i = 0; i < components.size(); ++i, ++component)
	{
		std::vector<const SingularOrPluralGramInfo *> gi;

		ParseWord(*component, *rodChislo, GetFioPart(combination[i]), gi);

		result.push_back(new ParsedPartFIO(*component, gi));
	}
}

void RussianDeclensor::ParseWord(
	const MultiPartDecoratedWord& word,
	RodChislo rodChislo,
	FioPart fioPart,
	std::vector<const SingularOrPluralGramInfo*>& gi) const
{
	gi.reserve(word.PartCount());

	bool onlyLastPartIsInflected = OnlyLastPartIsInflected(word, fioPart)
		|| fioDict.Contains(word.Lemma());

	for (auto part = word.Parts().begin(); part != word.Parts().end(); ++part)
	{
		bool isUninflected = onlyLastPartIsInflected && (gi.size() < word.PartCount() - 1);

		gi.push_back(isUninflected
			? Uninflected(rodChislo, false)->GetSingular()
			: ParseWord(part->Lemma(), rodChislo, fioPart));
	}
}

const SingularOrPluralGramInfo* RussianDeclensor::ParseWord(
	const std::tstring& lemma,
	RodChislo rodChislo,
	FioPart fioPart) const
{
	const SingularOrPluralGramInfo* gramInfo = GetGramInfo(lemma, fioPart, rodChislo);

	if (gramInfo) return gramInfo;

	Paradigms::Row* endings = GetEndingsAuto(lemma, rodChislo == SingMasc, fioPart);

	if (endings == nullptr)
	{
		// NULL возвращается ТОЛЬКО для "нестандартных" фамилий (Лев).
		// Просклонять ее как имя.
		return ParseWord(lemma, rodChislo, FioPart_GivenName);
	}

	return gramInfoFactory().GetGramInfo(endings,
		endings->endingsType == EndingsType_Adjectival
		&& std::tstring(_T("ой")).compare(endings->endings[Padeg_I]) == 0);
}

Paradigms::Row* RussianDeclensor::GetEndingsAuto(
	const std::tstring& word,
	bool masculine,
	FioPart fioPart)
{
	return (fioPart == FioPart_FamilyName)
		? GetFamilyNameEndings(word, masculine)
		: GetGivenNameEndings(word, masculine);
}

Paradigms::Row* RussianDeclensor::GetFamilyNameEndings(
	const std::tstring& familyName,
	bool masculine)
{
	if (!masculine && Common::HasEndings(familyName, _T("ая")))
	{
		// Большинство слов на -АЯ адъективного склонения (Лучинская, 
		// Осадчая), но бывают и субстантивного (типа: Ая, Рая, Тая, Ашая, Вирсая).

		// Имён на -ЯЯ я не нашёл, а фамилий очень мало: Заболотняя, Пидсадняя.

		return &Paradigms::AdjectivalSingularFeminine;
	}
	else if (masculine && Common::HasEndings(familyName, _T("ой")))
	{
		return &Paradigms::AdjectivalSingularMasculineEndingStressed;
	}
	else if (masculine && Common::HasEndings(familyName, _T("ый")))
	{
		return &Paradigms::AdjectivalSingularMasculineStemStressed;
	}
	else if (masculine && Common::HasEndings(familyName, _T("ов,ин,цын"))) // Арын Болат Айдарулы
	{
		return &Paradigms::MasculineFamilyName;
	}
	else if (!masculine && Common::HasEndings(familyName, _T("ова,ина,цына")))
	{
		return &Paradigms::PronomialSingularFeminine;
	}
	else if (Common::HasEndings(familyName, _T("ых")))
	{
		return &Paradigms::Uninflected;
	}
	else
	{
		return nullptr;
	}
}

Paradigms::Row* RussianDeclensor::GetGivenNameEndings(
	const std::tstring& givenName,
	bool masculine)
{
	if (Common::HasEndings(givenName, _T("а")))
	{
		return &Paradigms::Noun1stDecl;
	}
	else if (Common::IsVowel(*givenName.rbegin()))
	{
		return &Paradigms::Uninflected;
	}
	else
	{
		// На согласную, Ь или Ъ.
		if (masculine)
		{
			return &Paradigms::Noun2ndDeclMasculine;
		}
		else
		{
			return &Paradigms::Uninflected;
		}
	}
}

const SingularOrPluralGramInfo* RussianDeclensor::GetGramInfo(
	const std::tstring& lemma,
	FioPart fioPart,
	RodChislo rodChislo) const
{
	// Женские фамилии на согласную не склоняются.
	if (rodChislo == SingFem
		&& fioPart != FioPart_GivenName
		&& !Common::EndsWithOneOf(lemma, _T("а,я")))
	{
		return Uninflected(SingFem, true)->GetSingular();
	}

	// TODO
	//// Омонимичны топонимам и фильтруются таблицей Омонимы в ФИО.mdb.
	//if (lemma == "дания") return new GramInfo (Парадигмы.СтандартныеОкончания.Noun1stDecl, RodChislo.SingFem, true /* ударное */, false /*беглая*/);
	//if (lemma == "алтай" && rodChislo == RodChislo.SingFem) return new GramInfo (Парадигмы.СтандартныеОкончания.Uninflected, RodChislo.SingFem, true, false);

	if (fioPart == FioPart_FamilyName
		&& !fioDict.Contains(lemma, FioPart_FamilyName)
		&& Common::HasFamilyNameSuffix(lemma))
	{
		return nullptr;
	}

	const GramInfo* gi = dictionary.FindHomonym(lemma);

	if (gi)
	{
		// Фамилии мужского имеют тот же Byte, что и притяжательные отцов, дедов.
		if (gi->GetEndings()->GetEndingsType() == EndingsType_Pronomial
			&& gi->rodChislo == SingMasc)
		{
			// исправить окончание предложного.
			return gramInfoFactory().MasculineFamilyName();
		}

		return gi->GetSingular();
	}

	return FindInCommonDictionary(lemma, rodChislo, fioPart);
}

bool RussianDeclensor::OnlyLastPartIsInflected(
	const MultiPartDecoratedWord& word,
	FioPart fioPart) const
{
	const std::vector<DecoratedWord>& parts = word.Parts();

	switch (fioPart)
	{
	case FioPart_FamilyName:
		return false;

	case FioPart_GivenName:
		for (size_t i = 0; i < parts.size() - 1; ++i)
		{
			if (fioDict.Contains(parts[i].Lemma(), FioPart_GivenName)) return false;
		}
		return true;

	case FioPart_Patronymic:

		// В "Саид-Ахметович" Саид не склоняется, 
		// но в "Игоревич-Олегович" склоняются обе parts.
		for (size_t i = 0; i < parts.size() - 1; ++i)
		{
			if (Common::HasEndings(parts[i].Lemma(), _T("ыч,на"))) return false;
		}
		return true;
	}

	throw std::exception(); // "Invalid fioPart"
}

bool RussianDeclensor::IsSpecialWord(const MultiPartDecoratedWord& word)
{
	// TODO
	return false;
}

bool RussianDeclensor::IsCommonNoun(const std::tstring& lemma) const
{
	return Common::HasFamilyNameSuffix(lemma)
		&& dictionary.ContainsFullLemma(lemma);
}

bool RussianDeclensor::NotAHomonym(const std::tstring& lemma) const
{
	// TODO
	return dictionary.FindHomonym(lemma) == nullptr;
}

bool RussianDeclensor::IsCommonNoun(
	const std::list<MultiPartDecoratedWord>& components) const
{
	return components.size() == 1
		&& IsCommonNoun(components.front().LastPart())
		&& NotAHomonym(components.front().LastPart());
}

bool RussianDeclensor::IsCommonNounWithFamilyNameSuffix(const std::tstring& lemma) const
{
	return Common::HasFamilyNameSuffix(lemma)
		&& dictionary.ContainsFullLemma(lemma);
}

bool RussianDeclensor::ContainsWordsFromFioDictionary(
	const std::list<MultiPartDecoratedWord>& components) const
{
	std::list<MultiPartDecoratedWord>::const_iterator component = components.begin();

	for (int i = 0; i < 3 && component != components.end(); ++i, ++component)
	{
		std::tstring lowerCaseWord = component->Lemma();

		if (IsCommonNounWithFamilyNameSuffix(lowerCaseWord))
		{
			if ((*component->Parts().begin()).GetCaps().IsFirstCap()) return true;
			continue;
		}

		if (fioDict.Contains(lowerCaseWord) && NotAHomonym(lowerCaseWord)) return true;
	}

	return false;
}

bool RussianDeclensor::AllWordsLowercase(
	const std::list<MultiPartDecoratedWord>& components)
{
	for (auto word = components.begin(); word != components.end(); ++word)
	{
		if (!word->IsSameCaps(Caps::AllLower)) return false;
	}

	return true;
}

bool RussianDeclensor::WordLikeIbn(const MultiPartDecoratedWord& word)
{
	if (word.PartCount() != 1) return false;

	const std::tstring& s = word.LastPart();

	if (s.compare(_T("ибн")) == 0) return true;
	if (s.compare(_T("ди")) == 0) return true;
	if (s.compare(_T("сан")) == 0) return true;
	if (s.compare(_T("батоно")) == 0) return true;

	return false;
}

bool RussianDeclensor::IsFirstCap(const MultiPartDecoratedWord& word)
{
	return word.IsSameCaps(Caps::FirstCap) ||
		(word.FirstPart().length() == 1 && word.IsSameCaps(Caps::AllCaps)); // IsInitial
}

bool RussianDeclensor::AllWordsTitleCase(const std::list<MultiPartDecoratedWord>& components)
{
	for (auto word = components.begin(); word != components.end(); ++word)
	{
		if (WordLikeIbn(*word)) continue;

		if (!(IsFirstCap(*word) || (word == components.begin()
			&& components.size() > 1
			&& word->IsSameCaps(Caps::AllCaps)))) return false;
	}

	return true;
}

bool RussianDeclensor::CaseSuggestsName(const std::list<MultiPartDecoratedWord>& components)
{
	return AllWordsInCaps(components) || AllWordsTitleCase(components);
}

bool RussianDeclensor::AllWordsInCaps(const std::list<MultiPartDecoratedWord>& components)
{
	for (auto word = components.begin(); word != components.end(); ++word)
	{
		if (WordLikeIbn(*word)) continue;

		if (!(word->IsSameCaps(Caps::AllCaps))) return false;
	}

	return true;
}

bool RussianDeclensor::EndsInOvoEvo(const std::list<MultiPartDecoratedWord>& components)
{
	return AllWordsTitleCase(components) &&
		Common::EndsWithOneOf((*components.begin()).LastPart(), _T("ово,ево,ёво,ино,ыно"));
}

bool RussianDeclensor::ContainsPatronymic(const std::list<MultiPartDecoratedWord>& components)
{
	std::list<MultiPartDecoratedWord>::const_iterator word = ++components.begin();

	for (int i = 1; word != components.end() && i < 3; ++word, ++i)
	{
		std::tstring lowerCaseWord = word->LastPart();

		if (Common::EndsWithOneOf(
			lowerCaseWord, _T("ович,евич,овна,евна,инична,ыч,оглы,кызы")))
		{
			return true;
		}
	}

	return false;
}

bool RussianDeclensor::DoesNotLookLikeFio(
	const std::list<MultiPartDecoratedWord>& components)
{
	for (auto word = components.begin(); word != components.end(); ++word)
	{
		// "Соединённое Королевство, Агентство, Ойло Союзное, Подмосковье, Мытищи"
		if (Common::EndsWithOneOf(word->LastPart(), _T("ое,ство,овье,щи"))) return true;
	}

	return false;
}

bool RussianDeclensor::HasFamilyNameSuffix(const std::tstring& lowerCaseWord) const
{
	return Common::HasFamilyNameSuffix(lowerCaseWord)
		&& !IsCommonNounWithFamilyNameSuffix(lowerCaseWord);
}

bool RussianDeclensor::ContainsWordsWithFamilyNameSuffix(
	const std::list<MultiPartDecoratedWord>& components) const
{
	for (auto word = components.begin(); word != components.end(); ++word)
	{
		if (HasFamilyNameSuffix(word->FirstPart())) return true;
	}

	return false;
}

bool RussianDeclensor::IsOnlyOneWordWithFamilyNameSuffix(
	const std::list<MultiPartDecoratedWord>& components) const
{
	if (components.size() != 1) return false;
	std::tstring lowerCaseWord = (*components.begin()).LastPart();
	return HasFamilyNameSuffix(lowerCaseWord);
}

bool RussianDeclensor::LooksLikeFio(
	const std::list<MultiPartDecoratedWord>& components) const
{
	if (components.size() == 1)
	{
		// "она, Господь"
		if (IsSpecialWord(*components.begin())) return false;

		if ((*components.begin()).PartCount() == 1)
		{
			std::tstring lemma = (*components.begin()).LastPart();

			// Эти фамилии омонимичны местоименным прилагательным.
			// В случае, если они употреблены отдельно, считаем их фамилиями.
			if (lemma.compare(_T("иванов")) == 0) return true;
			if (lemma.compare(_T("петров")) == 0) return true;
		}
	}

	if (IsCommonNoun(components)) return false;

	// TODO
	//if (Топоним (components)) return false;

	if (ContainsWordsFromFioDictionary(components)
		&& (AllWordsLowercase(components) || CaseSuggestsName(components)))
		return true;

	if (EndsInOvoEvo(components)) return true;

	if (ContainsPatronymic(components)) return true;

	if (Common::EndsWithOneOf(
		(*components.begin()).LastPart()
		, _T("ы,ые,ие,ое,ее,ск,ки"))) return false;

	if (DoesNotLookLikeFio(components)) return false;

	if (components.size() == 1 && (*components.begin()).PartCount() > 1 &&
		dictionary.ContainsFullLemma((*components.begin()).Lemma()))
		return false;

	if ((components.size() == 3
		|| ContainsWordsWithFamilyNameSuffix(components))
		&& AllWordsTitleCase(components)) return true;

	// Одно слово типа "иванов" - это скорее всего familyName.
	if (IsOnlyOneWordWithFamilyNameSuffix(components)) return true;

	return components.size() == 1
		&& !NotAHomonym((*components.begin()).Lemma())
		&& (*(*components.begin()).Parts().begin()).GetCaps().IsFirstCap();
}

RussianDeclensor::WordWithHyphens::WordWithHyphens(
	const MultiPartDecoratedWord& word,
	const std::vector<GC::Ptr<Word>>& parts) : word(word), parts(parts)
{
}

bool RussianDeclensor::WordWithHyphens::IsNonAdjectivalNoun() const
{
	return parts.front()->IsNonAdjectivalNoun();
}

bool RussianDeclensor::WordWithHyphens::IsPossessiveAdjective() const
{
	return parts.front()->IsPossessiveAdjective();
}

RodChislo RussianDeclensor::WordWithHyphens::GetRodChislo() const
{
	return parts.front()->GetRodChislo();
}

bool RussianDeclensor::WordWithHyphens::IsAnimate() const
{
	return parts.front()->IsAnimate();
}

bool RussianDeclensor::WordWithHyphens::UseNA() const
{
	return parts.front()->UseNA();
}

std::tstring RussianDeclensor::WordWithHyphens::Lemma() const
{
	// ???? что тут вернуть нужно.
	return word.Lemma();
}

bool RussianDeclensor::WordWithHyphens::AppendTo(
	PhraseBuilder& phraseBuilder,
	std::list<std::tstring>::const_iterator& c,
	bool plural) const
{
	for (std::vector<GC::Ptr<Word>>::size_type i = 0; i < parts.size(); i++)
	{
		if (!parts[i]->AppendTo(phraseBuilder, c, plural))
		{
			return false;
		}

		if (i < parts.size() - 1)
		{
			phraseBuilder.Add(*c++);
		}
	}

	return true;
}

RussianDeclensor::WordMorpher::WordMorpher(
	const std::tstring& lemma,
	size_t endingLength,
	bool endingStressed,
	bool volatileVowel)
	: lemma(lemma)
	, endingStressed(endingStressed)
	, volatileVowel(volatileVowel || Common::EndsWith(lemma, _T("ья")))
{
	// определяем тип основы
	bool stemTypeIsKnown = false;

	if (endingLength == 0
		&& (Common::EndsWith(lemma, _T('ь')) || Common::EndsWith(lemma, _T('й'))))
	{
		endingLength = 1;
		stemType = Soft;
		stemTypeIsKnown = true;
	}
	else if (endingLength == 1
		&& (Common::EndsWith(lemma, _T("не"))
			|| lemma == _T("баре")
			|| lemma == _T("бояре")))
	{
		// римляне, армяне, бояре...
		stemType = Firm;
		stemTypeIsKnown = true;
	}

	const size_t stemLength = lemma.length() - endingLength;
	this->stem = lemma.substr(0, stemLength);
	this->lemmaEnding = lemma.substr(stemLength);
	this->lemmaEndingLength = endingLength;

	if (!stemTypeIsKnown)
	{
		if (lemma.length() <= endingLength)
		{
			throw std::runtime_error("lemma.length() <= endingLength");
		}
		switch (lemma[lemma.length() - 1 - endingLength])
		{
		case _T('ж'): case _T('ш'):
		case _T('ч'): case _T('щ'):
		case _T('г'): case _T('к'): case _T('х'):
			stemType = Mixed;
			break;
		case _T('ц'):
			stemType = Firm;
			break;
		default:
			stemType = StartsWithSoftening(lemmaEnding)
				? Soft : Firm;
			break;
		}
	}
}

bool RussianDeclensor::WordMorpher::IsAmiException(const std::tstring& stem)
{
	static const TCHAR* x[] = {
		_T("люд"), _T("дет"), _T("лошад")
	};
	for (size_t i = 0; i < sizeof(x) / sizeof(*x); ++i)
	{
		if (stem.compare(x[i]) == 0) return true;
		std::tstring polu = _T("полу");
		polu += x[i];
		if (stem.compare(polu) == 0) return true;
	}
	return false;
}

std::tstring RussianDeclensor::WordMorpher::AddEnding(std::tstring ending) const
{
	// основа может измениться, так что
	// скопируем её в локальную переменную
	std::tstring stem = this->stem;

	if (ending.empty())
	{
		if (stemType == Soft)
		{
			ending = EndsWithConsonant(stem) ? _T("ь") :
				(Common::EndsWith(stem, _T('й')) ? _T("") /*секвой*/ : _T("й"));
		}
	}
	else
	{
		// обозначение мягкости основы и чередование 
		// о/е/ё в зависимости от ударения
		switch (ending[0])
		{
		case _T('а'):
			if (stemType == Soft) ending[0] = _T('я');
			if (ending.compare(_T("ями")) == 0 && IsAmiException(stem))
			{
				ending = _T("ьми");
			}
			break;
		case _T('у'):
			if (stemType == Soft) ending[0] = _T('ю');
			break;
		case _T('ы'):
			if (stemType != Firm) ending[0] = _T('и');
			break;
		case _T('о'):
			TCHAR c;
			if (EndsWithSibilantOrTse(stem))
			{
				c = endingStressed ? _T('о') : _T('е');
			}
			else if (stemType == Soft)
			{
				c = endingStressed ? _T('ё') : _T('е');
			}
			else
			{
				c = _T('о');
			}
			ending[0] = c;
			break;
		case _T('е'):
			if ((Common::EndsWith(stem, _T('и'))
				// в односложных основах (кий, Лия) оставляем -е:
				&& ContainsVowel(stem.substr(0, stem.length() - 1))
				&& !endingStressed)
				||
				Common::EndsWith(stem, _T("забыть"))) // забытье, полузабытье
			{
				ending = _T("и"); // дат. и предл.
			}
			break;
		}
	}
	if (volatileVowel && stem.length() >= 2)
	{
		TCHAR c1 = stem[stem.length() - 1];
		TCHAR c2 = stem[stem.length() - 2];

		if (FormHasFleetingVowel(ending))
		{
			// определить, какую гласную вставлять
			// и подготовить основу
			TCHAR vowel;
			if (c1 == _T('ь'))
			{
				// гостья, ущелье, статья, питьё и др.
				if (Common::EndsWith(stem, _T("семь")) // семьи
					|| Common::EndsWith(stem, _T("ружь")) // ружья
					|| Common::EndsWith(stem, _T("судь"))) // судьи
				{
					vowel = _T('е');
				}
				else
				{
					vowel = endingStressed ? _T('е') : _T('и');
				}
				// заменить Ь на Й (судья - судей)
				stem[stem.length() - 1] = _T('й');
				ending = _T("");
			}
			else if (c2 == _T('й') || c2 == _T('ь'))
			{
				if (Common::EndsWith(stem, _T("яйц")))
				{ // яйцо
					vowel = _T('и');
				}
				else if (Common::EndsWith(stem, _T("серьг")))
				{ // серьга
					vowel = _T('ё');
				}
				else if (Common::EndsWith(stem, _T("тьм")))
				{ // тьма как _T('множество')
					vowel = _T('е'); // Р.мн. _T("тем")
				}
				else
				{ // шпилька, письмо, чайка, кайма, кольцо и др.
					vowel = (c1 == _T('ц')) ? _T('е') : endingStressed ? _T('ё') : _T('е');
				}
				// убрать c2 (Ь или Й)
				stem.erase(stem.length() - 2, 1);
			}
			else if (IsGuttural(c2) || (IsGuttural(c1) && !IsSibilantOrTse(c2)))
			{
				// кукла, окно, сказка, ведёрко и др.
				if (Common::EndsWith(stem, _T("кочерг")))
				{ // кочерги
					vowel = _T('ё');
				}
				else
				{
					vowel = _T('о');
				}
			}
			else
			{
				if (Common::EndsWith(stem, _T("шестерн")) // шестерня
					|| Common::EndsWith(stem, _T("сёстр")) // сёстры
					|| Common::EndsWith(stem, _T("сестр"))) // сёстры через Е
				{
					vowel = _T('ё');
				}
				else if (Common::EndsWith(stem, _T("деревн")) // деревни
					|| Common::EndsWith(stem, _T("корчм"))) // корчмы
				{
					// деревни - схема ударения е, окончание 
					// ударно в косвенных множественного
					vowel = _T('е');
				}
				else if (Common::EndsWith(stem, _T("дн")))
				{
					vowel = _T('о'); // дно - донья
				}
				else
				{
					vowel = (c1 == _T('ц')) ? _T('е') :
						(endingStressed ? (IsSibilantOrTse(c2)
							? _T('о') : _T('ё')) : _T('е'));
				}
			}
			// вишня - вишен, читальня - читален и т.п.
			if (stemType == Soft && c1 == _T('н'))
			{
				if (!Common::EndsWith(stem, _T("барышн")) &&
					!Common::EndsWith(stem, _T("боярышн")) &&
					!Common::EndsWith(stem, _T("кухн")) &&
					!Common::EndsWith(stem, _T("деревн")))
				{
					ending = _T(""); // основа отвердевает
				}
			}
			if (vowel == _T('ё'))
			{
				// удалить другие Ё из основы этой формы
				for (size_t i = 0; i < stem.length(); ++i)
				{
					if (stem[i] == _T('ё'))
					{
						stem[i] = _T('е');
					}
				}
			}
			// вставить гласную
			stem.insert(stem.length() - 1, 1, vowel);
		}
		else if (!Slogovoe(lemmaEnding) && Slogovoe(ending))
		{
			RemoveFleetingVowel(stem);
		}
	}

	//if (trial_expired ()) return stem + this->lemmaEnding;

	return stem += ending; // надо бы просто +, но в VC6&7 STL какой-то баг
}

bool RussianDeclensor::WordMorpher::FormHasFleetingVowel(const std::tstring& ending) const
{
	TCHAR c2 = lemma[lemma.length() - lemmaEndingLength - 2];
	TCHAR c1 = lemma[lemma.length() - lemmaEndingLength - 1];

	return BeginsWithVowel(lemmaEnding)
		&& !BeginsWithVowel(ending)
		&& !IsVowel(c2)
		&& !IsVowel(c1);
}

bool RussianDeclensor::WordMorpher::BeginsWithVowel(const std::tstring& s)
{
	return !s.empty() && IsVowel(s.front());
}

bool RussianDeclensor::WordMorpher::trial_expired()
{
	tm expiry;
	expiry.tm_year = 2013 - 1900;
	expiry.tm_mon = 1; // 0=January
	expiry.tm_mday = 1;
	expiry.tm_hour = 0;
	expiry.tm_min = 0;
	expiry.tm_sec = 0;
	expiry.tm_isdst = 1;

	time_t t = mktime(&expiry);

	time_t current_time = time(nullptr);

	return (current_time > t);
}

void RussianDeclensor::WordMorpher::RemoveFleetingVowel(std::tstring& stem)
{
	if (stem.length() < 3) return;

	TCHAR c3 = stem[stem.length() - 3];
	TCHAR c2 = stem[stem.length() - 2];
	TCHAR c1 = stem[stem.length() - 1];

	//     if (stem.length () > 3)
	//     {
	//TCHAR c4 = stem [stem.length () - 4];

	//         // С одной стороны: игрец, беглец, борщец
	//         // С другой:
	//         // участок, костёр, свёрток, выкормок, столбец, 
	//         // волчец, волчок, фламандец, должок, желток, поползень, 
	//         // выползка, загвоздка, 
	//         if (!IsVowel (c3) && !IsVowel (c4) && !((c4=='з' && c3=='д') || (c4=='с' && c3=='т') || c4=='р' || c4=='л' || c4=='н')) return;
	//     }

	// убрать гласную
	if (c1 == _T('е') || c1 == _T('и'))
	{
		// улья, воробья, муравья, третьего, птичьего
		// заменить гласную на разделительный Ь
		stem[stem.length() - 1] = _T('ь');
	}
	else if (stem.length() > 2)
	{
		// зверька, льда, пальца
		// орла, кашля, конца
		bool soften = (c3 == _T('л') && IsSoftening(c2))
			|| ((c2 == _T('е') || c2 == _T('ё')) && c1 == _T('к')
				&& !IsSibilantOrTse(c3)); // ребёночка, казачка (есть еще familyName Казачёк)

		if (soften) stem.insert(stem.length() - 2, 1, _T('ь'));

		// убрать гласную
		stem.erase(stem.length() - 2, 1);

		if (IsVowel(c3))
		{
			// боец - бойца
			stem.insert(stem.length() - 1, 1, _T('й'));
		}
	}
}

bool RussianDeclensor::WordMorpher::Slogovoe(const std::tstring& ending)
{
	return ending.length() != 0 // нулевое не является слоговым
		&& ending[0] != _T('й')
		&& ending[0] != _T('ь'); // -ь, -ью
}

bool RussianDeclensor::WordMorpher::IsSoftening(TCHAR c)
{
	static const std::tstring s = _T("еияюё");
	return s.find(c) != std::tstring::npos;
}

bool RussianDeclensor::WordMorpher::StartsWithSoftening(const std::tstring& s)
{
	return !s.empty() && IsSoftening(s[0]);
}

bool RussianDeclensor::WordMorpher::IsConsonant(TCHAR c)
{
	static const std::tstring s = _T("бвгджзклмнпрстфхцчшщ");
	return s.find(c) != std::tstring::npos;
}

bool RussianDeclensor::WordMorpher::IsVowel(TCHAR c)
{
	static const std::tstring s = _T("аяоёиыуюэе");
	return s.find(c) != std::tstring::npos;
}

bool RussianDeclensor::WordMorpher::EndsWithConsonant(const std::tstring& s)
{
	return !s.empty() && IsConsonant(*s.rbegin());
}

bool RussianDeclensor::WordMorpher::IsSibilantOrTse(TCHAR c)
{
	static const std::tstring s = _T("жшщчц");
	return s.find(c) != std::tstring::npos;
}

bool RussianDeclensor::WordMorpher::EndsWithSibilantOrTse(const std::tstring& s)
{
	return !s.empty() && IsSibilantOrTse(*s.rbegin());
}

bool RussianDeclensor::WordMorpher::ContainsVowel(const std::tstring& s)
{
	return s.find_first_of(_T("аяоёиыуюэе")) != std::tstring::npos;
}

bool RussianDeclensor::WordMorpher::IsGuttural(TCHAR c)
{
	static const std::tstring s = _T("гкх");
	return s.find(c) != std::tstring::npos;
}

std::tstring RussianDeclensor::SingleWordWithGramInfo::GetForm2(
	const SingularOrPluralGramInfo* gramInfo,
	const std::tstring& lemma,
	Padeg padeg,
	bool isAnimate)
{
	if (padeg == Padeg_V && gramInfo->GetEndings()->AccAnimEqualsGen())
	{
		padeg = isAnimate
			? Padeg_R
			: Padeg_I;
	}

	std::tstring ending = (*gramInfo->GetEndings())[padeg];

	// "в забытьи" - исключение
	if (padeg == Padeg_M
		&& (Common::EndsWith(lemma, _T("забытьё"))
			|| Common::EndsWith(lemma, _T("забытье"))))
	{
		ending = _T("и");
	}

	std::tstring form = WordMorpher(
		lemma,
		(*gramInfo->GetEndings())[Padeg_I].length(),
		gramInfo->EndingStressed(),
		gramInfo->FleetingVowel()).AddEnding(ending);

	// "жилье, копье" - не надо ё
	if ((*gramInfo->GetEndings())[Padeg_I].compare(_T("о")) == 0
		&& gramInfo->EndingStressed()
		&& Common::EndsWith(lemma, _T('е')))
	{
		size_t i = form.find_last_of(_T('ё'));

		if (i != std::tstring::npos)
		{
			form[i] = _T('е');
		}
	}

	if (!Common::ContainsVowel(form)) return _T(""); // мгл

	return form;
}

RussianDeclensor::SingleWordWithGramInfo::SingleWordWithGramInfo(
	const SingularOrPluralGramInfo* gi,
	const DecoratedWord& word) : gi(gi), word(word)
{
}

std::tstring RussianDeclensor::SingleWordWithGramInfo::GetFormByCase(
	Padeg padeg,
	bool animate) const
{
	std::tstring form = GetForm2(gi, word.Lemma(), padeg, animate);
	if (form.empty()) return form;
	return word.ApplyTo(form);
}

bool RussianDeclensor::SingleWordWithGramInfo::PaulcalIsSingular() const
{
	return gi->GetEndings()->GetEndingsType() == EndingsType_NounLike;
}

RussianDeclensor::DictionaryWord::DictionaryWord(
	const MultiPartDecoratedWord& word,
	const GramInfo* gi) : word(word), gi(gi)
{
}

std::tstring RussianDeclensor::DictionaryWord::Lemma() const
{
	return word.LastPart();
}

RodChislo RussianDeclensor::DictionaryWord::GetRodChislo() const
{
	return gi->rodChislo;
}

bool RussianDeclensor::DictionaryWord::IsPossessiveAdjective() const
{
	return gi->GetEndings()->GetEndingsType() == EndingsType_Pronomial;
}

bool RussianDeclensor::DictionaryWord::IsNonAdjectivalNoun() const
{
	EndingsType endingsType = gi->GetEndings()->GetEndingsType();

	return gi->IsNoun
		&& endingsType != EndingsType_Adjectival
		&& endingsType != EndingsType_Pronomial;
}

bool RussianDeclensor::DictionaryWord::IsAnimate() const
{
	return gi->IsAnimate;
}

bool RussianDeclensor::DictionaryWord::UseNA() const
{
	return gi->Na;
}

bool RussianDeclensor::DictionaryWord::AppendTo(
	PhraseBuilder& phraseBuilder,
	std::list<std::tstring>::const_iterator& separator,
	bool plural) const
{
	for (size_t i = 0; i < word.PartCount() - 1; ++i)
	{
		phraseBuilder.Add(word.Parts()[i].DecoratedLemma());
		phraseBuilder.Add(*separator++);
	}

	DecoratedWord lemma = *word.Parts().rbegin();

	if (plural)
	{
		const Endings* pluralEndings = gi->GetEndingsPlural();
		if (pluralEndings == nullptr) return false;
	}

	phraseBuilder.Add(new SingleWordWithGramInfo(
		plural ? gi->GetPlural() : gi->GetSingular(),
		plural ? lemma.Clone(GetPluralLemma()) : lemma));

	return true;
}

std::tstring RussianDeclensor::DictionaryWord::GetPluralLemma() const
{
	const Endings* pluralEndings = gi->GetEndingsPlural();

	std::tstring pluralLemma =
		gi->Alteration ? ApplyAlterations(word.LastPart()) :
		WordMorpher(
			word.LastPart(),
			(*gi->GetEndings())[Padeg_I].length(),
			gi->EndingIsStressed(),
			gi->GetSingular()->FleetingVowel()).AddEnding((*pluralEndings)[Padeg_I]);

	// звезда - звёзды, село - сёла
	if (gi->Yo && gi->EndingIsStressed() && !gi->EndingIsStressedPlural())
	{
		size_t i = pluralLemma.find_last_of(_T('е'));

		if (i != std::tstring::npos)
		{
			pluralLemma[i] = _T('ё');
		}
	}

	return pluralLemma;
}

std::tstring RussianDeclensor::DictionaryWord::ApplyAlterations(std::tstring singularLemma)
{
	if (singularLemma == _T("бесёнок")
		|| singularLemma == _T("бесенок")) return _T("бесенята");
	if (singularLemma == _T("чертёнок")
		|| singularLemma == _T("чертенок")) return _T("чертенята");

	static const TCHAR* s[] = {
		_T("онок"),_T("ата"),
		_T("енок"),_T("ята"),
		_T("ёнок"),_T("ята"),
		_T("оночек"),_T("атки"),
		_T("еночек"),_T("ятки"),
		_T("ёночек"),_T("ятки"),
		_T("ин"),_T("е"),
		_T("г"),_T("зья"),
		_T("к"),_T("чья"),
		_T("ь"),_T("ья"),
		_T(""),_T("ья") };

	for (unsigned i = 0; i < sizeof(s) / sizeof(*s); i += 2)
	{
		std::tstring singularSide = s[i];
		std::tstring pluralSide = s[i + 1];

		// перо, дерево...
		if (Common::EndsWith(singularLemma, _T("о"))) singularSide += _T("о");

		if (singularLemma.length() - 1 < singularSide.length()) continue;

		// клин
		if (Common::EndsWith(singularLemma, singularSide))
		{
			std::tstring lemma =
				singularLemma.substr(0, singularLemma.length() - singularSide.length());
			if (singularSide.length() > 1 && !Common::ContainsVowel(lemma)) continue;
			return lemma + pluralSide;
		}
	}

	return singularLemma;
}

bool RussianDeclensor::ContainsVowel(const std::tstring& s)
{
	const std::tstring vowels = _T("аяоёиыуюэе");
	return s.find_first_of(vowels) != std::tstring::npos;
}

void RussianDeclensor::FindDeclensionForSimpleWord(
	const MultiPartDecoratedWord& word,
	bool first,
	std::list<GC::Ptr<Word>>& result) const
{
	// TODO
	//// Обработка сложносокращённых типа "завскладом, завотделением, завкадрами".
	//if (word.PartCount == 1 && Common.СловоТипаЗавскладом (word.FirstPart))
	//{
	//    ДобавитьНесклоняемое (word, result, RodChislo.SingMasc);
	//    ДобавитьНесклоняемое (word, result, RodChislo.SingFem);

	//    return result;
	//}

	// поискать это слово в словаре особых слов
	specialDictionary.Find(word, result);
	if (!result.empty()) return;

	// если не нашли, то поискать в основном словаре
	std::list<const GramInfo *> gis;
	dictionary.Find(word.Lemma(), first, gis);

	for (auto gi = gis.begin(); gi != gis.end(); ++gi)
	{
		result.push_back(GC::Ptr<Word>(new DictionaryWord(word, *gi)));
	}
}

RussianDeclensor::GramInfoGenders::GramInfoGenders()
{
	gramInfoFactory = GramInfoFactory();
	gramInfoFem = GramInfo(
		RodChislo::SingFem,
		true,
		&gramInfoFactory.GetEndings()[0],
		false,
		false,
		&gramInfoFactory.GetEndings()[0],
		false,
		true,
		false,
		false,
		false);

	gramInfoMasc = GramInfo(
		RodChislo::SingMasc,
		true,
		&gramInfoFactory.GetEndings()[0],
		false,
		false,
		&gramInfoFactory.GetEndings()[0],
		false,
		true,
		false,
		false,
		false);

	gramInfoNeut = GramInfo(
		RodChislo::SingNeut,
		true,
		&gramInfoFactory.GetEndings()[0],
		false,
		false,
		&gramInfoFactory.GetEndings()[0],
		false,
		true,
		false,
		false,
		false);

	gramInfoPlural = GramInfo(
		RodChislo::Plural,
		true,
		&gramInfoFactory.GetEndings()[0],
		false,
		false,
		&gramInfoFactory.GetEndings()[0],
		false,
		true,
		false,
		false,
		false);
}

const GramInfo* RussianDeclensor::GramInfoGenders::GramInfoFem() const
{
	return &gramInfoFem;
}

const GramInfo* RussianDeclensor::GramInfoGenders::GramInfoMasc() const
{
	return &gramInfoMasc;
}

const GramInfo* RussianDeclensor::GramInfoGenders::GramInfoNeut() const
{
	return &gramInfoNeut;
}

const GramInfo* RussianDeclensor::GramInfoGenders::GramInfoPlural() const
{
	return &gramInfoPlural;
}

GC::Ptr<RussianDeclensor::DictionaryWord> RussianDeclensor::Indeclinable(
	RodChislo rodChiso,
	const MultiPartDecoratedWord& word) const
{
	switch (rodChiso)
	{
	case SingMasc:
		return GC::Ptr<DictionaryWord>(
			new DictionaryWord(word, IndeclinableGramInfoGlobal.GramInfoMasc()));
	case SingFem:
		return GC::Ptr<DictionaryWord>(
			new DictionaryWord(word, IndeclinableGramInfoGlobal.GramInfoFem()));
	case SingNeut:
		return GC::Ptr<DictionaryWord>(
			new DictionaryWord(word, IndeclinableGramInfoGlobal.GramInfoNeut()));
	case Plural:
		return GC::Ptr<DictionaryWord>(
			new DictionaryWord(word, IndeclinableGramInfoGlobal.GramInfoPlural()));
	default:
		throw std::invalid_argument("RodChislo out of expected range (Indeclinable)");
	}

}

void RussianDeclensor::AddIndeclinable(
	const MultiPartDecoratedWord& word,
	std::list<GC::Ptr<Word>>& result,
	RodChislo rodChislo) const
{
	result.push_back(Indeclinable(rodChislo, word));
}

void RussianDeclensor::IndeclinableWithoutGender(
	const MultiPartDecoratedWord& word,
	std::list<GC::Ptr<Word>>& result) const
{
	AddIndeclinable(word, result, RodChislo::SingMasc);
	AddIndeclinable(word, result, RodChislo::SingFem);
	AddIndeclinable(word, result, RodChislo::SingNeut);
	AddIndeclinable(word, result, RodChislo::Plural);
}

RussianDeclensor::SpecialDictionary::SpecialWord::SpecialWord(
	RodChislo rodChislo,
	bool isAnimate,
	bool IsNoun,
	const std::vector<std::tstring>& singularForms,
	const std::vector<std::tstring>& pluralForms)
	: singularForms(singularForms)
	, plural(pluralForms.empty() ? NULL : new SpecialWord(Plural, isAnimate,
		IsNoun, pluralForms, std::vector<std::tstring>()))
	, rodChislo(rodChislo)
	, IsNoun(IsNoun)
	, isAnimate(isAnimate)
{
}

const std::tstring& RussianDeclensor::SpecialDictionary::SpecialWord::Lemma() const
{
	return singularForms[0];
}

bool RussianDeclensor::SpecialDictionary::SpecialWord::IsNonAdjectivalNoun() const
{
	return IsNoun;
}

RodChislo RussianDeclensor::SpecialDictionary::SpecialWord::GetRodChislo() const
{
	return rodChislo;
}

bool RussianDeclensor::SpecialDictionary::SpecialWord::IsAnimate() const
{
	return isAnimate;
}

const RussianDeclensor::SpecialDictionary::SpecialWord*
RussianDeclensor::SpecialDictionary::SpecialWord::GetPlural() const
{
	return plural.GetP();
}

std::tstring RussianDeclensor::SpecialDictionary::SpecialWord::GetFormByCase(
	Padeg padeg,
	bool anim) const
{
	std::tstring form = GetFormByCaseInternal(padeg, anim);
	return form == _T("-") ? std::tstring(_T("")) : form;
}

std::tstring RussianDeclensor::SpecialDictionary::SpecialWord::GetFormByCaseInternal(
	Padeg padeg,
	bool anim) const
{
	switch (padeg)
	{
	case Padeg_I: return singularForms[0];
	case Padeg_R: return singularForms[1];
	case Padeg_D: return singularForms[2];
	case Padeg_V: return singularForms[3].empty()
		? anim
		? singularForms[1] // родительный
		: singularForms[0] // именительный
		: singularForms[3];
	case Padeg_T: return singularForms[4];
	case Padeg_P: return singularForms[5];
	case Padeg_M: return singularForms[5];
	default: throw std::runtime_error("Invalid Padeg.");
	}
}

RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::DecoratedSpecialWord(
	MultiPartDecoratedWord word,
	const SpecialWord& specialWord) : word(word), specialWord(specialWord)
{
}

bool RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::IsNonAdjectivalNoun() const
{
	return specialWord.IsNonAdjectivalNoun();
}

RodChislo RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::GetRodChislo() const
{
	return specialWord.GetRodChislo();
}

bool RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::IsAnimate() const
{
	return specialWord.IsAnimate();
}

bool RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::IsPossessiveAdjective() const
{
	return false;
}

bool RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::AppendTo(
	PhraseBuilder& phraseBuilder,
	std::list<std::tstring>::const_iterator& separator,
	bool plural) const
{
	const DecoratedSpecialWord* word = plural
		? GetPlural()
		: this;

	if (!word) return false;

	phraseBuilder.Add(word);

	return true;
}

RussianDeclensor::SpecialDictionary::DecoratedSpecialWord*
RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::GetPlural() const
{
	const SpecialWord* plural = specialWord.GetPlural();

	return plural == nullptr ? 0 : new DecoratedSpecialWord(word, *plural);
}

std::tstring RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::GetFormByCase(
	Padeg padeg,
	bool isAnimate) const
{
	std::tstring form = specialWord.GetFormByCase(padeg, isAnimate);
	if (form.empty()) return _T("");
	return word.Parts()[0].ApplyTo(form);
}

bool RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::PaulcalIsSingular() const
{
	return IsNonAdjectivalNoun();
}

bool RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::UseNA() const
{
	// Пока среди особых нет с предлогом НА.
	return false;
}

std::tstring RussianDeclensor::SpecialDictionary::DecoratedSpecialWord::Lemma() const
{
	return specialWord.Lemma();
}

bool RussianDeclensor::SpecialDictionary::Less::operator()(
	const SpecialWord& w1,
	const SpecialWord& w2) const
{
	return w1.Lemma() < w2.Lemma();
}

RussianDeclensor::SpecialDictionary::SpecialDictionary()
{
	{
		std::stringstream file(
			std::string(static_cast<char *>(specialDict), specialDict_size));

		LemmaReader lemmaReader(file, LemmaReader::CompareFromBeginning);

		while (true)
		{
			unsigned char b = file.get();

			if (file.eof()) break;

			RodChislo rodChislo = static_cast<RodChislo>(b & 3);

			bool plural = (b & 4) != 0;

			std::vector<std::tstring> singularForms = ReadForms(lemmaReader);

			std::vector<std::tstring> pluralForms = plural
				? ReadForms(lemmaReader)
				: std::vector<std::tstring>();

			bool anim = (b & 8) != 0;

			Add(true, rodChislo, anim, singularForms, pluralForms);

			if (plural)
			{
				Add(true, Plural, anim, pluralForms, std::vector<std::tstring>());
			}
		}
	}

	// Инициализировать особые прилагательные.
	{
		std::stringstream file(std::string(specialAdjDict, specialAdjDict_size));

		LemmaReader lemmaReader(file, LemmaReader::CompareFromBeginning);

		while (true)
		{
			char c = file.get();

			if (file.eof()) break;

			file.putback(c);

			std::vector<std::tstring> singularMasculineForms = ReadForms(lemmaReader);
			std::vector<std::tstring> singularFeminineForms = ReadForms(lemmaReader);
			std::vector<std::tstring> singularNeuterForms = ReadForms(lemmaReader);
			std::vector<std::tstring> pluralForms = ReadForms(lemmaReader);

			Add(false, Plural, false, pluralForms, std::vector<std::tstring>()); // такой порядок нужен для "всё/все"
			Add(false, SingMasc, false, singularMasculineForms, pluralForms);
			Add(false, SingFem, false, singularFeminineForms, pluralForms);
			Add(false, SingNeut, false, singularNeuterForms, pluralForms);
		}
	}
}

std::vector<std::tstring> RussianDeclensor::SpecialDictionary::ReadForms(
	LemmaReader& lemmaReader)
{
	unsigned char b = lemmaReader.Stream().get();

	std::vector<std::tstring> singularForms(6);

	for (size_t i = 0; i < singularForms.size(); ++i)
	{
		unsigned char unused;

		std::tstring lemma = lemmaReader.GetNext(&unused);

		singularForms[i] = ((b & (1 << i)) == 0) ? std::tstring(_T("-")) : lemma;
	}

	return singularForms;
}

void RussianDeclensor::SpecialDictionary::Add(
	bool IsNoun,
	RodChislo rodChislo,
	bool anim,
	std::vector<std::tstring> singularForms,
	const std::vector<std::tstring>& pluralForms)
{
	const std::tstring& lemma = singularForms[0];
	bool YoMakesADifference = !lemma.empty() && lemma[0] == _T('Ё') && lemma[1] == _T('!');
	if (YoMakesADifference) singularForms[0] = singularForms[0].substr(2);

	SpecialWord word(rodChislo, anim, IsNoun, singularForms, pluralForms);

	words.insert(word);

	if (word.Lemma().find(_T('ё')) != std::tstring::npos && !YoMakesADifference)
	{
		Add(IsNoun, rodChislo, anim, RemoveYo(singularForms), RemoveYo(pluralForms));
	}
}

std::vector<std::tstring> RussianDeclensor::SpecialDictionary::RemoveYo(
	const std::vector<std::tstring>& forms)
{
	std::vector<std::tstring> r(forms.size());

	for (size_t i = 0; i < r.size(); ++i)
	{
		std::tstring s = ReplaceYo(forms[i]);
		r[i] = s;
	}

	return r;
}

std::tstring RussianDeclensor::SpecialDictionary::ReplaceYo(std::tstring s)
{
	for (size_t i = 0; i < s.length(); ++i)
	{
		if (s[i] == _T('ё')) s[i] = _T('е');
	}

	return s;
}

void RussianDeclensor::SpecialDictionary::Find(
	const MultiPartDecoratedWord& word,
	std::list<GC::Ptr<Word>>& result) const
{
	std::tstring lemma = word.Lemma();
	std::vector<std::tstring> forms(1);
	forms[0] = lemma;
	SpecialWord val(Plural, false, false, forms, forms);

	Words::const_iterator it = words.find(val);

	while (it != words.end() && it->Lemma() == lemma)
	{
		result.push_back(GC::Ptr<Word>(new DecoratedSpecialWord(word, *it++)));
	}
}

bool RussianDeclensor::NeedToFindDeclensionForFirstWordOnly(
	const MultiPartDecoratedWord& word)
{
	if (word.PartCount() > 2)
	{
		if (word.Parts()[1].DecoratedLemma() == _T("на") // Ростов-на-Дону, Франкфурт-на-Майне
			|| word.Parts()[1].DecoratedLemma() == _T("дель")) // Сантьяго-дель-Эстеро
		{
			return true;
		}
	}

	if (word.PartCount() == 2 && word.Parts()[1].DecoratedLemma() == _T("то")) return true;
	return false;
}

template <typename T, typename R>
std::vector<R> RussianDeclensor::VectorOf(
	const std::vector<T> vec,
	std::function<R(T)> func)
{
	std::vector<R> result;
	result.reserve(vec.size());

	for (auto v : vec)
	{
		result.push_back(func(v));
	}

	return result;
}

GC::Ptr<RussianDeclensor::Word> RussianDeclensor::ChooseWord(
	RodChislo rodChislo,
	const std::list<GC::Ptr<Word>>& words)
{
	for (auto word : words)
	{
		if ((1 << word->GetRodChislo()) == rodChislo)
		{
			return word;
		}
	}

	throw std::runtime_error("No words");
}

GC::Ptr<RussianDeclensor::Word> RussianDeclensor::FindWordOfTheSpecifiedNumber(
	const std::list<GC::Ptr<Word>>& words,
	bool plural)
{
	for (auto word : words)
	{
		if ((word->GetRodChislo() == RodChislo::Plural) == plural)
		{
			return word;
		}
	}

	return nullptr;
}

void RussianDeclensor::AnalyzeHyphenatedWord(
	const MultiPartDecoratedWord& word,
	bool first,
	std::list<GC::Ptr<Word>>& result) const
{
	if (word.PartCount() < 2)
	{
		return;
	}

	auto parts = word.Parts();

	std::list<std::tstring> switchParts{ _T("им"), _T("ам"), _T("ан"), _T("ин") };
	for (auto it = std::next(parts.begin()); it != parts.end(); ++it)
	{
		if (std::find(
			switchParts.begin(),
			switchParts.end(),
			it->Lemma()) != switchParts.end())
		{
			IndeclinableWithoutGender(word, result);
			return;
		}
	}

	auto func = [](const DecoratedWord& w) { return w.Lemma(); };

	std::function<bool(const std::vector<std::tstring>&)> inflectedOnlyLastPart =
		[&](const std::vector<std::tstring>& parts)
	{
		std::tostringstream ss;
		std::copy(
			parts.begin(), parts.end() - 1,
			std::ostream_iterator<std::tstring, TCHAR>(ss, _T("-")));
		ss << *parts.rbegin();
		std::tstring joinedParts = ss.str();
		if (dictionary.ContainsFullLemma(joinedParts))
		{
			return true;
		}

		return Common::InflectedOnlyLastPart(parts, inflectedOnlyLastPart);
	};

	if (Common::InflectedOnlyLastPart(
		VectorOf<DecoratedWord, std::tstring>(word.Parts(), func),
		inflectedOnlyLastPart))
	{
		return;
	}

	std::vector<std::list<GC::Ptr<Word>>> variants(parts.size());
	std::vector<unsigned> attributes(parts.size());
	bool ambiguity = false;

	for (std::vector<DecoratedWord>::size_type i = 0; i < parts.size(); i++)
	{
		FindDeclensionForSimpleWord(MultiPartDecoratedWord(
			std::vector<DecoratedWord>{parts[i]}),
			first,
			variants[i]);

		if (variants[i].size() == 0) { return; }
		attributes[i] = GetUniqueAttributes(variants[i]);
		if (variants[i].size() > 1) { ambiguity = true; }
	}

	std::vector<GC::Ptr<Word>> wordParts;

	if (ambiguity)
	{
		unsigned intersection = attributes[0];
		for (std::vector<DecoratedWord>::size_type i = 1; i < parts.size(); i++)
		{
			intersection &= attributes[i];
			if (intersection == 0) { break; }
		}

		if (intersection > 0)
		{
			wordParts = std::vector<GC::Ptr<Word>>(parts.size());

			unsigned singleIntersection = 0;
			for (unsigned i = 0, j = 1; i < RodChislo::RodChislo_Count; i++)
			{
				singleIntersection = intersection & j;
				if (singleIntersection)
				{
					break;
				}

				j <<= 1;
			}

			for (std::vector<DecoratedWord>::size_type i = 0; i < parts.size(); ++i)
			{
				wordParts[i] = RussianDeclensor::ChooseWord(
					static_cast<RodChislo>(singleIntersection),
					variants[i]);
			}
		}
		else
		{
			bool allSingle = true;
			bool allPlural = true;
			std::vector<GC::Ptr<Word>> partsSingular(parts.size());
			std::vector<GC::Ptr<Word>> partsPlural(parts.size());

			for (std::vector<DecoratedWord>::size_type i = 0; i < parts.size(); ++i)
			{
				partsSingular[i] =
					RussianDeclensor::FindWordOfTheSpecifiedNumber(variants[i], false);
				partsPlural[i] =
					RussianDeclensor::FindWordOfTheSpecifiedNumber(variants[i], true);
			}

			if (allSingle)
			{
				wordParts = partsSingular;
			}
			else if (allPlural)
			{
				wordParts = partsPlural;
			}
			else
			{
				return;
			}
		}
	}
	else
	{
		wordParts = std::vector<GC::Ptr<Word>>(parts.size());
		for (std::vector<DecoratedWord>::size_type i = 0; i < parts.size(); i++)
		{
			wordParts[i] = variants[i].front();
		}
	}

	wordParts = std::vector<GC::Ptr<Word>>(parts.size());
	for (std::vector<DecoratedWord>::size_type i = 0; i < parts.size(); i++)
	{
		wordParts[i] = variants[i].front();
	}

	result.push_back(GC::Ptr<Word>(new WordWithHyphens(word, wordParts)));
	return;
}

void RussianDeclensor::FindDeclension(
	const MultiPartDecoratedWord& word,
	bool first,
	std::list<GC::Ptr<Word>>& result) const
{
	// Часто этому условию удовлетворяют предлоги (в, с, к).
	if (!ContainsVowel(word.LastPart())) return;

	if (dictionary.ContainsFullLemma(word.Lemma()))
	{
		FindDeclensionForSimpleWord(word, first, result);
		return;
	}

	if (NeedToFindDeclensionForFirstWordOnly(word))
	{
		std::list<GC::Ptr<Word>> declensionWords;
		std::vector<DecoratedWord> lemma;
		lemma.push_back(word.Parts()[0]);
		FindDeclensionForSimpleWord(MultiPartDecoratedWord(lemma), first, declensionWords);

		for (auto it = declensionWords.begin(); it != declensionWords.end(); ++it)
		{
			result.push_back(GC::Ptr<Word>(new WordLikeRostovNaDonu(word, *it)));
		}
		return;
	}

	//if (word.FirstPart.StartsWith ("пол") && (Common.EndsWithOneOf (word.LastPart, "ого,его")/* && word.Length > 8*/))
	//{
	//    // полпервого, полвторого, полтретьего, ... пол-одиннадцатого, полдвенадцатого
	//    LinkedList <Word> result = new LinkedList <Word> ();
	//    ДобавитьНесклоняемое (word, result, RodChislo.SingNeut);
	//    return result;
	//}

	AnalyzeHyphenatedWord(word, first, result);
	if (!result.empty()) { return; }

	FindDeclensionForSimpleWord(word, first, result);
}

unsigned RussianDeclensor::GetUniqueAttributes(const std::list<GC::Ptr<Word>>& words)
{
	unsigned attr = 0;

	for (auto word = words.begin(); word != words.end(); ++word)
	{
		attr |= (1 << static_cast<unsigned>((*word)->GetRodChislo()));
	}

	return attr;
}

bool RussianDeclensor::IsStopWord(const MultiPartDecoratedWord& word)
{
	// TODO 
	return false;
}

bool RussianDeclensor::CanBeGenitive(const MultiPartDecoratedWord& word) const
{
	return dictionary.CanBeGenitive(word.LastPart());
}

void RussianDeclensor::AnalyseNaric(
	std::list<MultiPartDecoratedWord>& components,
	std::list<std::tstring>& separators,
	Attributes flags,
	RodChislo* rodChislo,
	bool* isAnimate,
	bool* prepositionNA,
	std::string& fioCombination,
	std::list<GC::Ptr<IParsedPart>>& result) const
{
	std::list<MultiPartDecoratedWord>::const_iterator word = components.begin();

	if (!Common::ContainsVowel(word->LastPart()))
	{
		// вероятно, аббревиатура
		return;
	}

	std::list<std::list<GC::Ptr<Word>>> matches;

	matches.push_back(std::list<GC::Ptr<Word>>());

	FindDeclension(*word, true, *matches.begin());

	if ((*matches.begin()).empty())
	{
		return;
	}

	// Битовая маска, хранящая набор пар признаков (род, число), 
	// по которым согласуются все просмотренные до сих пор слова.  
	// Продолжаем просмотр, пока не кончатся слова или не встретится слово, 
	// которое не согласуется со словами до него.
	unsigned attributes = GetUniqueAttributes(*matches.begin());

	for (++word; word != components.end(); ++word)
	{
		if (IsStopWord(*word)) break;

		if (attributes != (1 << Plural) && CanBeGenitive(*word))
		{
			std::list<MultiPartDecoratedWord>::const_iterator nextWord = word;

			++nextWord;

			if (nextWord != components.end() && CanBeGenitive(*nextWord))
			{
				// следующее слово тоже в родительном
				break;
			}
		}

		std::list<GC::Ptr<Word>> currentMatches;

		FindDeclension(*word, false, currentMatches);

		unsigned atts = attributes;

		atts &= GetUniqueAttributes(currentMatches);

		if (atts == 0) break;

		attributes = atts;

		// значит, текущее слово согласуется с предыщущими

		// сохранить варианты анализа этого слова
		matches.push_back(currentMatches);
	}

	// выбираем из пар {род, число} ту, 
	// что принадлежит первому слову (так что моль склоняется правильно)
	*rodChislo = GetRodChislo(attributes, *matches.begin());

	return SecondPass(
		matches,
		components,
		separators,
		flags,
		*rodChislo,
		isAnimate,
		prepositionNA,
		result);
}

RodChislo RussianDeclensor::GetRodChislo(
	unsigned attributes,
	const std::list<GC::Ptr<Word>>& matches)
{
	assert(attributes != 0);

	for (auto i = matches.begin(); i != matches.end(); ++i)
	{
		RodChislo rodChislo = (*i)->GetRodChislo();

		if (((1 << static_cast<unsigned>(rodChislo)) & attributes) != 0)
		{
			return rodChislo;
		}
	}

	throw std::runtime_error("GetRodChislo");
}

void RussianDeclensor::SecondPass(
	const std::list<std::list<GC::Ptr<Word>>>& matches,
	const std::list<MultiPartDecoratedWord>& components,
	const std::list<std::tstring>& separators,
	Attributes flags,
	RodChislo rodChislo,
	bool* isAnimate,
	bool* prepositionNA,
	std::list<GC::Ptr<IParsedPart>>& words)
{
	// Из оставшихся вариантов разбора выбираем первые 
	// согласующиеся с этими родом и числом;
	// в склоняемые включается только одно существительное,
	// остальные идут в uninflected, например: звон колоколов,
	// паттерны проектирования (хотя эти существительные не в именительном,
	// они зачастую всё равно распознаются как существительные).
	Word* rootWord = nullptr;
	bool animateMet = false;

	std::list<std::tstring>::const_iterator delimNode = separators.begin();

	std::list<MultiPartDecoratedWord>::const_iterator word = components.begin();

	for (auto currentWordMatches = matches.begin();
		currentWordMatches != matches.end(); ++currentWordMatches)
	{
		// Пропустить столько разделителей, сколько частей в слове.
		for (size_t i = 0; i < word->PartCount(); ++i) ++delimNode;

		++word;

		// из вариантов разбора текущего слова
		// находим первый, согласующийся с найденной 
		// в первом проходе парой (род, число)
		Word* wordThatMatches = GetWordThatMatches(*currentWordMatches, rodChislo);

		// "Комната Ленина"
		if (rootWord != nullptr && wordThatMatches->IsPossessiveAdjective()) break;

		if (wordThatMatches->IsNonAdjectivalNoun())
		{
			if (rootWord != nullptr) break;
			rootWord = wordThatMatches;
		}

		if (wordThatMatches->IsAnimate()) animateMet = true;

		if (delimNode->find_first_of(_T(",-(")) != std::tstring::npos
			&& word != components.end() && !Common::Participle(word->LastPart())) // "данайцы, приносящие дары"
		{
			rootWord = nullptr;
		}

		words.push_back(GC::Ptr<IParsedPart>(wordThatMatches));
	}

	*isAnimate = rootWord != nullptr ? rootWord->IsAnimate() : animateMet;

	// Топонимы все неодушевленные, даже Вождь Пролетариата.
	*isAnimate &= (flags & Attribute_Toponym) == 0;

	*prepositionNA = rootWord == nullptr ? false : rootWord->UseNA();
}

RussianDeclensor::Word* RussianDeclensor::GetWordThatMatches(
	const std::list<GC::Ptr<Word>>& words,
	RodChislo rodChislo)
{
	for (auto word = words.begin(); word != words.end(); ++word)
	{
		if ((*word)->GetRodChislo() == rodChislo) return &**word;
	}

	throw std::runtime_error("GetWordThatMatches: error.");
}

RussianDeclensor::AnalysisMethod RussianDeclensor::ChooseParser(
	const std::list<MultiPartDecoratedWord>& components,
	Attributes attributes) const
{
	if ((attributes & Attribute_Fio) != 0 && (attributes & Attribute_Naric) != 0)
	{
		throw std::runtime_error(
			"Mutually excusive attributes specified: Attribute_Fio and Attribute_Naric.");
	}

	if ((attributes & (Attribute_Fio | Attribute_Masc | Attribute_Fem)) != 0)
	{
		// Указание на мужской или женский род означает, что это имя.
		return &RussianDeclensor::AnalyseFio;
	}

	if ((attributes & Attribute_Naric) != 0)
	{
		return &RussianDeclensor::AnalyseNaric;
	}

	return (LooksLikeFio(components))
		? &RussianDeclensor::AnalyseFio
		: &RussianDeclensor::AnalyseNaric;
}

bool RussianDeclensor::Flatten(PhraseBuilder& phraseBuilder,
	const std::list<GC::Ptr<IParsedPart>>& parsedComponents,
	const std::list<MultiPartDecoratedWord>& components,
	const std::list<std::tstring>& separators,
	bool plural)
{
	std::list<std::tstring>::const_iterator separator = separators.begin();

	phraseBuilder.Add(*separator++);

	std::list<MultiPartDecoratedWord>::const_iterator component = components.begin();

	for (auto parsedComponent = parsedComponents.begin();
		parsedComponent != parsedComponents.end();
		++parsedComponent)
	{
		if (!(*parsedComponent)->AppendTo(phraseBuilder, separator, plural))
		{
			return false;
		}

		phraseBuilder.Add(*separator++);
		++component;
	}

	// Если проанализировано меньше составляющих, чем было на входе,
	// записать оставшиеся составляющие в исходном виде, не склоняя.
	for (; component != components.end(); ++component)
	{
		for (size_t part = 0; part < component->PartCount(); ++part)
		{
			if (part > 0) phraseBuilder.Add(*separator++);
			phraseBuilder.Add(component->Parts()[part].DecoratedLemma());
		}

		phraseBuilder.Add(*separator++);
	}

	return true;
}

RussianDeclensor::RussianDeclensor()
{
}

RussianDeclensor::~RussianDeclensor()
{
}

Phrase* RussianDeclensor::Analyse(const std::tstring& s, Attributes attributes) const
{
	Phrase* phrasePtr = AnalyseInternal(s, attributes);

	//Exception searchException = {{s.c_str()}};

	//Exception * exceptionDictEnd = exceptionDict + exceptionDict_size;

	//Exception * it = std::lower_bound (exceptionDict, exceptionDictEnd, searchException, 
	//	[] (const Exception & e1, const Exception & e2) {return tstrcmp (e1.singular.nominative, e2.singular.nominative) < 0; });

	//if (it != exceptionDictEnd)
	//{
	//}

	return phrasePtr;
}

Phrase* RussianDeclensor::Indeclinable(const std::tstring& s, RodChislo rodChislo)
{
	std::list<GC::Ptr<const IPhrasePart>> invariablePart
	{
		GC::Ptr<const ::IPhrasePart>(new InvariablePart(s))
	};

	Phrase* pluralPhrase = new Phrase(
		invariablePart,
		RodChislo::Plural,
		false,
		false,
		nullptr
	);

	return new Phrase(
		invariablePart,
		rodChislo,
		false,
		false,
		pluralPhrase
	);
}

std::tstring RussianDeclensor::BuildStringFromWordsAndSep(
	size_t startIndex,
	const std::list<DecoratedWord>& words,
	const std::list<std::tstring>& separators)
{
	auto wordsIt = words.begin();
	auto sepIt = separators.begin();

	for (size_t i = 0; i < startIndex; i++)
	{
		wordsIt++;
		sepIt++;
	}

	std::tstring lemma;
	while (wordsIt != words.end())
	{
		lemma.append(*sepIt);
		lemma.append(wordsIt->GetCaps().GetOriginal());
		wordsIt++;
		sepIt++;
	}

	// Разделитлей всегда на 1 больше чем слов
	lemma.append(*sepIt);
	return lemma;
}

void RussianDeclensor::MoveRightPartToleftSeparator(
	size_t startIndex,
	std::list<DecoratedWord>& words,
	std::list<std::tstring>& separators)
{
	std::tstring lemma = BuildStringFromWordsAndSep(startIndex, words, separators);
	while (words.size() > startIndex)
	{
		words.pop_back();
		separators.pop_back();
	}

	separators.pop_back();
	separators.push_back(lemma);
}

Phrase* RussianDeclensor::MoveIndeclinable(
	std::list<DecoratedWord>& words,
	std::list<std::tstring>& separators) const
{
	if (words.size() < 2)
	{
		return nullptr;
	}

	auto sepShouldBeWhiteSpace = *++separators.begin();

	for (auto letter : sepShouldBeWhiteSpace)
	{
		if (letter != _T(' '))
		{
			return nullptr;
		}
	}

	std::tstring firstWordUpper = words.front().Lemma();
	Caps::ToUpper(firstWordUpper);

	// Аббревиатуры и наречия ==============================================================
	const std::set<std::tstring> abbreviationsAndAverbs{
		// Наречия не склоняются:
		_T("ВРЕМЕННО"), _T("ПРЕДЕЛЬНО"), _T("ОЧЕНЬ"), _T("СИЛЬНО"), _T("ВЕСЬМА"),

		// Сокращение не склоняется, остальное (предположительно ФИО) - склоняется.
		_T("ИП"), _T("ЧП"), _T("ПБОЮЛ")
	};

	if (abbreviationsAndAverbs.find(firstWordUpper) != abbreviationsAndAverbs.end())
	{
		*separators.begin() +=
			words.front().GetCaps().GetOriginal() + *std::next(separators.begin());
		separators.erase(std::next(separators.begin()));
		words.erase(words.begin());
		return nullptr;
	}

	// Город ===============================================================================
	if (firstWordUpper == _T("ГОРОД"))
	{
		std::tstring cityName = BuildStringFromWordsAndSep(1, words, separators);
		std::unique_ptr<Phrase> cityNameAnalysed(this->Analyse(cityName, Attribute_Toponym));

		if (cityNameAnalysed == nullptr)
		{
			MoveRightPartToleftSeparator(1, words, separators);
			return nullptr;
		}

		RodChislo cityNameGender = cityNameAnalysed->getRodChislo();
		if (cityNameGender == RodChislo::SingNeut || cityNameGender == RodChislo::Plural)
		{
			MoveRightPartToleftSeparator(1, words, separators);
			return nullptr;
		}
		else
		{
			std::unique_ptr<Phrase> phrase1(
				this->Analyse(words.front().Lemma(), static_cast<Attributes>(0)));
			auto newPhrase = new Phrase(phrase1, cityNameAnalysed);
			return newPhrase;
		}
	}

	// Село, деревня и т.д. ================================================================
	const std::set<std::tstring> onlyIfSameGender{
		_T("СЕЛО"), _T("ДЕРЕВНЯ"), _T("ХУТОР"), _T("РЕКА")
	};

	if (onlyIfSameGender.find(firstWordUpper) != onlyIfSameGender.end())
	{
		if (words.size() > 2)
		{
			MoveRightPartToleftSeparator(1, words, separators);
			return nullptr;
		}

		std::tstring geoTerm = BuildStringFromWordsAndSep(1, words, separators);

		std::unique_ptr<Phrase> genericWordAnalysed(
			this->Analyse(words.front().Lemma(),
				static_cast<Attributes>(0)));
		std::unique_ptr<Phrase> geoTermAnalysed(
			this->Analyse(geoTerm, Attributes::Attribute_Toponym));

		if (geoTermAnalysed != nullptr
			&& (genericWordAnalysed->getRodChislo() == geoTermAnalysed->getRodChislo()))
		{
			auto newPhrase = new Phrase(genericWordAnalysed, geoTermAnalysed);
			return newPhrase;
		}
		else
		{
			MoveRightPartToleftSeparator(1, words, separators);
			return nullptr;
		}
	}

	// Озеро, поселок, мыс и т.д ===========================================================
	const std::set<std::tstring> geoTerms{
		_T("БОЛОТО"), _T("БУХТА"), _T("ГОРЫ"), _T("ГОСУДАРСТВО"),
		_T("ДОЛИНА"), _T("ЗАЛИВ"), _T("ЗАСТАВА"), _T("ЗЕМЛЯ"),
		_T("КИШЛАК"), _T("КЛЮЧ"), _T("КОЛОДЕЦ"), _T("КОРОЛЕВСТВО"),
		_T("МЕСТЕЧКО"), _T("МЕСТОРОЖДЕНИЕ"), _T("МЫС"), _T("ОБЛАСТЬ"),
		_T("ОЗЕРО"), _T("ОКРУГ"), _T("ОСТРОВ"), _T("ПЕРЕВАЛ"),
		_T("ПЛАТО"), _T("ПЛОСКОГОРЬЕ"), _T("ПЛОТИНА"), _T("ПЛОЩАДЬ"),
		_T("ПОЛУОСТРОВ"), _T("ПОСЕЛОК"), _T("ПОСЁЛОК"), _T("ПРОВИНЦИЯ"),
		_T("ПРОЛИВ"), _T("ПРОМЫСЕЛ"), _T("РАЙОН"), _T("СЕЛЕНИЕ"),
		_T("СТАНЦИЯ"), _T("УРОЧИЩЕ"), _T("ХРЕБЕТ"), _T("ШТАТ"),
		_T("ПГТ")
	};

	if (geoTerms.find(firstWordUpper) != geoTerms.end())
	{
		int startIndex = 1;
		if (words.size() > 2)
		{
			MoveRightPartToleftSeparator(startIndex, words, separators);
			return nullptr;
		}

		std::tstring geoTerm = BuildStringFromWordsAndSep(startIndex, words, separators);
		bool isAdjective = Common::HasAdjectivalEnding(geoTerm.rbegin(), geoTerm.rend());
		std::unique_ptr<Phrase> geoTermAnalysed(
			this->Analyse(geoTerm, static_cast<Attributes>(0)));
		if (geoTermAnalysed != nullptr && !isAdjective && geoTermAnalysed->Fio() == nullptr)
		{
			MoveRightPartToleftSeparator(startIndex, words, separators);
		}

		return nullptr;
	}

	return nullptr;
}

Phrase* RussianDeclensor::AnalyseInternal(
	const std::tstring& s,
	Attributes attributes) const
{
	if (s == _T("Проектная документация без сметы и результаты инженерных изысканий"))
	{
		auto phrase1 = std::unique_ptr<Phrase>(
			Analyse(_T("Проектная документация без сметы и "), attributes));
		auto phrase2 = std::unique_ptr<Phrase>(
			Analyse(_T("результаты инженерных изысканий"), attributes));

		return new Phrase(phrase1, phrase2);
	}

	std::list<DecoratedWord> decoratedWords;
	std::list<std::tstring> separators;

	Tokenize(s, decoratedWords, separators);

	RemoveNonRussian(decoratedWords, separators);

	if (decoratedWords.empty()) return nullptr; // нет русских слов

	auto val = MoveIndeclinable(decoratedWords, separators);
	if (val != nullptr)
	{
		return val;
	}


	std::list<MultiPartDecoratedWord> components;

	JoinHypenatedWords(decoratedWords, separators, components);

	// Разбить входную строку на части, разделенные тире.
	std::list<TokenizedString> dashDelimitedStrings;

	std::list<MultiPartDecoratedWord> currentWordList;
	std::list<std::tstring> currentDelimList;

	Move(currentWordList, components);
	Move(currentDelimList, separators, currentWordList);

	while (!components.empty())
	{
		if (IsDash(*separators.begin()))
		{
			Move(currentDelimList, separators);
			separators.push_front(_T(""));
			dashDelimitedStrings.push_back(
				TokenizedString(currentWordList, currentDelimList));
			currentWordList.clear();
			currentDelimList.clear();
		}

		Move(currentWordList, components);
		Move(currentDelimList, separators, currentWordList);
	}

	Move(currentDelimList, separators);
	dashDelimitedStrings.push_back(TokenizedString(currentWordList, currentDelimList));

	assert(separators.empty());

	RodChislo rodChislo = SingMasc;
	bool animate = false;
	bool prepositionNA = false;

	bool first = true;

	PhraseBuilder phraseBuilder;
	PhraseBuilder phraseBuilderPlural;

	bool plural = true;

	std::string fioCombination;
	for (auto tokenizedString = dashDelimitedStrings.begin();
		tokenizedString != dashDelimitedStrings.end(); ++tokenizedString)
	{
		bool anim;
		RodChislo rch;

		std::list<GC::Ptr<IParsedPart>> comp;

		AnalysisMethod analyse = ChooseParser(tokenizedString->Words(), attributes);

		(this->*analyse)(
			tokenizedString->Words(), tokenizedString->Delims(), attributes,
			&rch, &anim, &prepositionNA, fioCombination, comp);

		if (comp.empty()) return nullptr; // какой-нибудь мусор вроде "р".

		if (anim) animate = true;
		if (first) rodChislo = rch;

		Flatten(
			phraseBuilder,
			comp,
			tokenizedString->Words(),
			tokenizedString->Delims(),
			false);

		if (plural)
		{
			if (!Flatten(
				phraseBuilderPlural,
				comp,
				tokenizedString->Words(),
				tokenizedString->Delims(),
				true))
			{
				plural = false;
			}
		}

		first = false;
	}

	auto it = abbreviations.find(currentWordList.front().FirstPart());
	if (it != abbreviations.end())
	{
		return Indeclinable(s, it->second);
	}

	Phrase* pluralArg = !plural
		? nullptr
		: new Phrase(phraseBuilderPlural.Parts(), Plural, animate, prepositionNA, nullptr);

	return new Phrase(
		phraseBuilder.Parts(),
		rodChislo,
		animate,
		prepositionNA,
		fioCombination,
		dashDelimitedStrings.back().Words(),
		pluralArg);
}

RussianDeclensor::WordLikeRostovNaDonu::WordLikeRostovNaDonu(
	const MultiPartDecoratedWord& lemma,
	const GC::Ptr<Word>& firstPart)
	: _lemma(lemma)
	, _firstPart(firstPart)
{
	_sLemma = _lemma.Lemma();
}

bool RussianDeclensor::WordLikeRostovNaDonu::IsNonAdjectivalNoun() const
{
	return _firstPart->IsNonAdjectivalNoun();
}

RodChislo RussianDeclensor::WordLikeRostovNaDonu::GetRodChislo() const
{
	return _firstPart->GetRodChislo();
}

bool RussianDeclensor::WordLikeRostovNaDonu::IsAnimate() const
{
	return _firstPart->IsAnimate();
}

bool RussianDeclensor::WordLikeRostovNaDonu::IsPossessiveAdjective() const
{
	return _firstPart->IsPossessiveAdjective();
}

bool RussianDeclensor::WordLikeRostovNaDonu::AppendTo(
	PhraseBuilder & phraseBuilder,
	std::list <std::tstring>::const_iterator & separator
	, bool plural) const
{
	if (!_firstPart->AppendTo(phraseBuilder, separator, plural))
	{
		return false;
	}


	for (size_t i = 1; i < _lemma.PartCount(); ++i)
	{
		phraseBuilder.Add(*separator++);
		phraseBuilder.Add(_lemma.Parts()[i].DecoratedLemma());
	}

	return true;
}

bool RussianDeclensor::WordLikeRostovNaDonu::UseNA() const
{
	return _firstPart->UseNA();
}

std::tstring RussianDeclensor::WordLikeRostovNaDonu::Lemma() const
{
	return _sLemma;
}

