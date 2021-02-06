// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "Dictionary.h"
#include <algorithm>

size_t Dictionary::ReverseLess::CharLess::IndexOf(TCHAR c) const
{
	// Функция IndexOf оптимизирована при помощи таблицы.
	return table[ToTableIndex(c)];
}

Dictionary::ReverseLess::CharLess::CharLess()
{
	// алфавит должен в точности совпадать 
	// с алфавитом в .NET версии
	const TCHAR alphabet[] =
		_T("аоуэыияюёейгкхжшщчцбпвфдтзсмнлрьъ-");

	for (size_t i = 0; i < sizeof(alphabet) / sizeof(*alphabet); ++i)
	{
		table[ToTableIndex(alphabet[i])] = static_cast<unsigned char>(i);
	}
}

bool Dictionary::ReverseLess::CharLess::operator()(TCHAR c1, TCHAR c2) const
{
	return IndexOf(c1) < IndexOf(c2);
}

unsigned Dictionary::ReverseLess::CharLess::ToTableIndex(TCHAR c)
{
	return
#ifdef _UNICODE
		static_cast<unsigned>
#else
		static_cast<unsigned char>
#endif
		(c);
}

bool Dictionary::ReverseLess::operator()(
	const std::tstring& s1,
	const std::tstring& s2) const
{
	static CharLess less;

	return lexicographical_compare(
		s1.rbegin(), s1.rend(),
		s2.rbegin(), s2.rend(), less);
}

Dictionary::Entry::Entry(
	const std::tstring& lemma,
	const Info* gi) : Lemma(lemma), GramInfo(gi)
{
}

Dictionary::SubstPlural::SubstPlural() : nomEnding(' ')
, genEnding(' ')
{
}

Dictionary::SubstPlural::SubstPlural(
	unsigned char nomEnding,
	unsigned char genEnding) : nomEnding(nomEnding), genEnding(genEnding)
{
}

std::tstring Dictionary::SubstPlural::operator[](Padeg c) const
{
	static const TCHAR* endings[]
		= { nullptr, nullptr, _T("ам"), nullptr, _T("ами"), _T("ах"), _T("ах") };
	static const TCHAR* nomStr[] = { _T("ы"), _T("а"), _T("е") };
	static const TCHAR* genStr[] = { _T(""), _T("ов"), _T("ей") };
	if (c == Padeg_I || c == Padeg_V) return nomStr[nomEnding];
	if (c == Padeg_R) return genStr[genEnding];
	return endings[c];
}

bool Dictionary::SubstPlural::AccAnimEqualsGen() const
{
	return true;
}

bool Dictionary::SubstPlural::IsNotInflected() const
{
	return false;
}

EndingsType Dictionary::SubstPlural::GetEndingsType() const
{
	return EndingsType_NounLike;
}

const Endings* Dictionary::DecodeGramInfoByte1(unsigned char b)
{
	b &= 0x3F;

	if ((b & 1) == 0)
	{
		const std::vector<ParameterlessParadigm>& singularEndings =
			gramInfoFactory.GetEndings();

		b >>= 1;

		// -1 для MasculineFamilyName
		if (b >= singularEndings.size() - 1) throw std::exception();

		return &singularEndings[b];
	}
	else
	{
		if (!pluralEndingsInitialised)
		{
			pluralEndingsInitialised = true;

			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
				{
					new(&pluralEndings[i][j]) SubstPlural(i, j);
				}
		}

		unsigned char nom = (b >> 1) & 3;
		unsigned char gen = (b >> 3) & 3;

		if (gen == 3) throw std::exception();

		return &pluralEndings[nom][gen];
	}
}

bool Dictionary::Less(const Entry& e1, const Entry& e2)
{
	return ReverseLess()(e1.Lemma, e2.Lemma);
}

Dictionary::Dictionary() : pluralEndingsInitialised(false)
{
	std::stringstream file(std::string(dict, dict_size));

	file.exceptions(file.exceptions() | std::ios::eofbit);

	int s0 = file.get();
	int s1 = file.get();
	size_t gisCount = s0 + (s1 << 8);

	gis.reserve(gisCount);

	for (size_t i = 0; i < gisCount; ++i)
	{
		unsigned char b0 = file.get();
		unsigned char b1 = file.get();
		unsigned char b2 = file.get();
		GramInfo gi(
			static_cast<RodChislo>(b0 & 3), // RodChislo 
			((b0 >> 2) & 1) != 0, // IsNoun
			DecodeGramInfoByte1(b1), // Endings
			((b0 >> 3) & 1) != 0, // EndingIsStressed 
			((b0 >> 4) & 1) != 0, // HasVolatileVowel
			b2 == 255 ? 0 : DecodeGramInfoByte1(b2), // EndingsPlural
			((b0 >> 6) & 1) != 0, // EndingIsStressedPlural 
			((b0 >> 5) & 1) != 0, // IsAnimate
			((b0 >> 7) & 1) != 0, // Alternation
			((b1 >> 7) & 1) != 0, // Na
			((b1 >> 6) & 1) != 0); // Yo

		gis.push_back(gi);
	}

	// прочитать количество статей
	size_t entryCount = Global::GetInt32(file);

	entries.reserve(entryCount);

	LemmaReader lemmaReader(file, LemmaReader::CompareFromEnd);

	for (size_t i = 0; i < entryCount; ++i) // цикл по статьям
	{
		unsigned char c = 0;

		const std::tstring& lemma = lemmaReader.GetNext(&c);

		// прочитать грамматическую информацию
		size_t gii = static_cast<unsigned char>(file.get()) + (c << 8);

		if (gii > gis.size()) throw std::exception();

		const Info* gi = 
			(gii == gis.size()) ? static_cast<const Info *>(&Indirect) : &gis[gii];

		// создать статью
		entries.push_back(Entry(lemma, gi));
	}

	// Прочитать homonyms.

	size_t count = Global::GetInt32(file);

	LemmaReader lemmaReader2(file, LemmaReader::CompareFromBeginning);

	for (size_t i = 0; i < count; ++i)
	{
		unsigned char b = 0;

		const std::tstring& lemma = lemmaReader2.GetNext(&b);

		homonyms[lemma] = &gis[file.get() + (b << 8)];
	}

	// инициализировать несклоняемые
	for (int rc = 0; rc < RodChislo_Count; ++rc)
	{
		for (int n = 0; n < 2; ++n)
		{
			const Endings* endings = &gramInfoFactory.GetEndings()[0];

			uninflected[rc][n] = GramInfo(static_cast<RodChislo>(rc),
				n == 0, // IsNoun 
				endings, false, false,
				endings, false,
				false, false, false, false);
		}
	}
}

const GramInfoFactory& Dictionary::getGramInfoFactory() const
{
	return gramInfoFactory;
}

const GramInfo* Dictionary::FindHomonym(const std::tstring& lemma) const
{
	std::map<std::tstring, const GramInfo *>::const_iterator i = homonyms.find(lemma);

	if (i == homonyms.end()) return nullptr;

	return i->second;
}

void Dictionary::Find(const std::tstring& lemma, bool first, ResultSet& result) const
{
	Entries::const_iterator entry0 
		= std::upper_bound(entries.begin(), entries.end(), Entry(lemma, nullptr), Less);

	// entry0 не может быть равно entries.begin (), 
	// т.к. первое слово в словаре "а" меньше любого другого.
	// а upper_bound выдает первый элемент, БОЛЬШИЙ данного.
	--entry0;

	if (entry0->Lemma != lemma)
	{
		if (first)
		{
			if (entry0->GramInfo->Nominative() == nullptr)
			{
				// например "лений"
				// продолжаем искать варианты склонения первого слова

				Entries::const_iterator j = entry0;
				do
				{
					--j;

					if (j->GramInfo->Nominative() != nullptr)
					{
						entry0 = j;
						break;
					}
				} while (j != entries.begin());
			}
		}

		AddResult(result, entry0, lemma);
	}
	else
	{
		// Cлово полностью совпадает со словарной статьёй.
		// Если есть другие статьи с такой же леммой, добавить их тоже.

		// Мы уже уменьшили entry0 на 1, 
		// поэтому сейчас она указывает на последний из равных.

		while (entry0 != entries.begin())
		{
			Entries::const_iterator entry1 = entry0 - 1;
			if (entry1->Lemma != lemma) break;
			entry0 = entry1;
		}

		// entry0 теперь указывает на первый из равных

		// Порядок добавления результатов важен, 
		// т.к. с помощью изменения порядка слов в zalizniak.txt
		// мы можем контролировать, какой из омографов имеет приоритет,
		// когда он употреблен без контекста
		// (например, моль - ж.р. имеет приоритет перед ж.р.)
		for (; entry0 != entries.end() && entry0->Lemma == lemma; ++entry0)
		{
			if (entry0->GramInfo->Nominative() == nullptr) continue;

			AddResult(result, entry0, lemma);
		}

		if (first && result.empty())
		{
			for (; entry0 != entries.begin(); --entry0)
			{
				if (entry0->GramInfo->Nominative() == nullptr) continue;
				AddResult(result, entry0, lemma);
				break;
			}
		}
	}
}

bool Dictionary::ContainsFullLemma(const std::tstring& lemma) const
{
	Entries::const_iterator entry0 
		= lower_bound(entries.begin(), entries.end(), Entry(lemma, nullptr), Less);

	return entry0 != entries.end() && entry0->Lemma.compare(lemma) == 0;
}

bool Dictionary::CanBeGenitive(const std::tstring& lemma) const
{
	Entries::const_iterator entry0 
		= lower_bound(entries.begin(), entries.end(), Entry(lemma, nullptr), Less);

	if (entry0 == entries.end() || entry0->Lemma != lemma)
	{
		--entry0;

		return CanBeGenitive(entry0);
	}

	// Cлово полностью совпадает со словарной статьёй.

	for (; entry0 != entries.end(); ++entry0)
	{
		if (entry0->Lemma != lemma) break;
		if (CanBeGenitive(entry0)) return true;
	}

	return false;
}

Gender Dictionary::GetGender(const std::tstring& lemma) const
{
	ResultSet result;

	Find(lemma, true, result);

	bool masculine = false;
	bool feminine = false;

	for (ResultSet::const_iterator gi = result.begin(); gi != result.end(); ++gi)
	{
		switch ((*gi)->rodChislo)
		{
		case SingMasc: masculine = true;
			break;
		case SingFem: feminine = true;
			break;
		case SingNeut:
		case Plural:
		case RodChislo_Count:
			break;
		}
	}

	if (masculine && feminine) return Gender_Unisex;
	if (masculine) return Gender_Masculine;
	if (feminine) return Gender_Feminine;

	return Gender_Unisex;
}

const GramInfo* Dictionary::Uninflected(RodChislo rodChislo, bool isNoun) const
{
	return &uninflected[static_cast<int>(rodChislo)][isNoun ? 0 : 1];
}

bool Dictionary::CanBeGenitive(Entries::const_iterator entry)
{
	const GramInfo* gi = entry->GramInfo->Nominative();
	if (gi == nullptr) return true;
	if (gi->GetEndings()->GetEndingsType() 
			== EndingsType_Adjectival 
			&& Common::EndsWith(entry->Lemma, _T("ой"))) return true; // молодой
	if (gi->rodChislo 
			== Plural 
			&& Common::EndsWithOneOf(entry->Lemma, _T("и,ы,а"))) return true;
	return false;
}

void Dictionary::AddResult(
	ResultSet& results,
	Entries::const_iterator result,
	const std::tstring& lemma) const
{
	const GramInfo* gramInfo = result->GramInfo->Nominative();

	if (!gramInfo) return;

	// Если лемма не оканчивается на окончание им.п.,
	// то этот вариант не подходит.
	// Эта проверка введена после найденного Володей
	// Аверкиным бага в слове "Сергея", которое неправильно
	// склонялось по адъективному типу.
	if (Common::HasEnding(lemma, (*gramInfo->GetEndings())[Padeg_I]))
	{
		results.push_back(gramInfo);
	}
	else
	{
		// например, "ие"
		// Признак "Сущ" ставим false, чтобы нормально склонялось "ответственный за"
		results.push_back(Uninflected(SingMasc, false));
		results.push_back(Uninflected(SingFem, false));
		results.push_back(Uninflected(SingNeut, false));
		results.push_back(Uninflected(Plural, false));
	}
}
