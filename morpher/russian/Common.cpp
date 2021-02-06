// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "Common.h"
#include <stdexcept>
#include <cstddef>

bool Common::IsVowel(TCHAR c) {
	return vowels.find(c) != std::tstring::npos;
}

bool Common::ContainsVowel(const std::tstring & s) {
	return s.find_first_of(vowels) != std::tstring::npos;
}

bool Common::IsConsonant(TCHAR c) {
	return std::tstring(_T("бвгджзйклмнпрстфхцчшщ")).find(c) != std::tstring::npos;
}

bool Common::HasFamilyNameSuffix(const std::tstring & lowerCaseWord) {
	return HasPronomialFamilyNameSuffix(lowerCaseWord)
		|| EndsWithOneOf(lowerCaseWord, _T("ко,их,ых,ян"));
}

bool Common::HasPronomialFamilyNameSuffix(const std::tstring & lowerCaseWord) {
	return EndsWithOneOf(lowerCaseWord, _T("ов,ев,ёв,ин,цын,ова,ева,ёва,ина,цына"));        
}

bool Common::EndsWith(
	std::tstring::const_reverse_iterator si,
	std::tstring::const_reverse_iterator s_rend,
	std::tstring::const_reverse_iterator ei,
	std::tstring::const_reverse_iterator e_rend) {
	if (e_rend - ei > s_rend - si) return false; // длина окончания больше длины слова

	while (ei != e_rend) {
		if (*si++ != *ei++) return false;
	}

	return true;
}

bool Common::EndsWith(const std::tstring & s, const std::tstring & ending) {
	return EndsWith(s.rbegin(), s.rend(), ending.rbegin(), ending.rend());
}

bool Common::EndsWith(const std::tstring & lemma, TCHAR c) {
	return !lemma.empty() && *lemma.rbegin() == c;
}

bool Common::EndsWithOneOf(
	std::tstring::const_reverse_iterator s,
	std::tstring::const_reverse_iterator rend, 
	const std::tstring & endings) {
	std::tstring::const_reverse_iterator eb = endings.rbegin();
	std::tstring::const_reverse_iterator ee = eb;

	for (; eb != endings.rend(); eb = ee) {
		while (ee != endings.rend() && *ee != _T(',')) ++ee;

		if (EndsWith(s, rend, eb, ee)) return true;

		if (ee != endings.rend()) ++ee; // skip the comma
	}

	return false;
}

bool Common::EndsWithOneOf(const std::tstring & s, const std::tstring & endings) {
	return EndsWithOneOf(s.rbegin(), s.rend(), endings);
}

bool Common::HasEnding2(
	const std::tstring & word,
	std::tstring::const_reverse_iterator eb,
	std::tstring::const_reverse_iterator ee) {
	if (!HasEnding(word, eb, ee)) return false;

	TCHAR lastLetterOfStem = word[word.length() - (ee - eb) - 1];

	if (word.length() == static_cast<size_t>(ee - eb) + 1) {
		// Всякие Ма, па не склоняются.
		// Чих, Лев, Рой не имеют окончаний -их, -ев, -ой.
		// Щи склоняются.
		if (Common::IsVowel(*(ee - 1)) 
			&& !Common::IsVowel(lastLetterOfStem) 
			&& lastLetterOfStem != _T('щ')) return false;
	}

	// Боа, Иов не имеют окончания -а и -ов.
	TCHAR firstLetterOfEnding = word[word.length() - (ee - eb)];
	if (Common::IsVowel(lastLetterOfStem) 
		&& (firstLetterOfEnding == _T('а') 
			|| firstLetterOfEnding == _T('о'))) return false;

	return true;
}

bool Common::Participle(
	std::tstring::const_reverse_iterator rbegin,
	std::tstring::const_reverse_iterator rend)
{
	if (!Common::HasAdjectivalEnding(rbegin, rend)) return false;

	return Common::HasParticipalSuffix(rbegin + 2, rend);
}

bool Common::Participle(const std::tstring& s)
{
	return Participle(s.rbegin(), s.rend());
}

bool Common::ReflexiveParticiple(const std::tstring& lowerCaseWord)
{
	if (lowerCaseWord.length() < 5) return false;

	if (!Common::EndsWith(lowerCaseWord, _T("ся"))) return false;

	return Participle(lowerCaseWord.rbegin() + 2, lowerCaseWord.rend());
}

bool Common::HasEndings(const std::tstring & lemma, const std::tstring & endings) {
	std::tstring::const_reverse_iterator eb = endings.rbegin();
	std::tstring::const_reverse_iterator ee = eb;

	for (; eb != endings.rend(); eb = ee) {
		while (ee != endings.rend() && *ee != _T(',')) ++ee;

		if (HasEnding2(lemma, eb, ee)) return true;

		if (ee != endings.rend()) ++ee; // skip the comma
	}

	return false;
}

bool Common::HasAdjectivalEnding(
	std::tstring::const_reverse_iterator s,
	std::tstring::const_reverse_iterator send) {
	return EndsWithOneOf(s, send, _T("ый,ий,ой,ая,яя,ое,ее,ые,ие"));
}

bool Common::HasParticipalSuffix(
	std::tstring::const_reverse_iterator s,
	std::tstring::const_reverse_iterator send) {
	return EndsWithOneOf(s, send, _T("ущ,ющ,ащ,ящ,ш"));
}

bool Common::HasEnding(
	const std::tstring & lemma,
	std::tstring::const_reverse_iterator eb,
	std::tstring::const_reverse_iterator ee) {
	std::ptrdiff_t endingLength = ee - eb;
	if (endingLength == 0) return true;
	if (endingLength >= (ptrdiff_t)lemma.length()) return false;

	if (lemma[lemma.length() - 1 - endingLength] == _T('-'))
		return false;

	// сравнение всех, кроме первой, букв окончания 
	// с соответствующими буквами леммы
	std::tstring::const_reverse_iterator si = lemma.rbegin();
	std::tstring::const_reverse_iterator ei = eb;
	for (int i = 0; i + 1 < endingLength; ++i) {
		if (*ei++ != *si++) return false;
	}

	// сравнение первой буквы окончания
	TCHAR c = *si;
	if (*ei == c) return true; // полное совпадение
	switch (*ei) {
	case _T('а'): return (c == _T('я'));
	case _T('о'): return (c == _T('е') || c == _T('ё'));
	case _T('у'): return (c == _T('ю'));
	case _T('ы'): return (c == _T('и'));
	}
	return false;
}

bool Common::HasEnding(const std::tstring & lemma, const std::tstring & ending) {
	return HasEnding(lemma, ending.rbegin(), ending.rend());
}

bool Common::InflectedOnlyLastPart(
	const std::vector<std::tstring> & parts,
	std::function<bool(const std::vector<std::tstring>&)> func) {
	if (parts.size() < 2) {
		throw std::invalid_argument("This method should be called only for complex words.");
	}

	std::list<std::tstring> indeclinable{ 
		_T("альфа"), _T("анти"), _T("арт"), _T("бета"), _T("бизнес"), _T("блиц"),
		_T("ватт"), _T("веб"), _T("вест"), _T("видео"), _T("вице"), _T("вакуум"),
		_T("вольт"),	_T("гала"), _T("гамма"), _T("генерал"), _T("грамм"),
		_T("дельта"), _T("дубль"), _T("зюйд"), _T("или"), _T("интернет"),
		_T("иссиня"), _T("ит"), _T("камер"), _T("киловатт"), _T("киловольт"),
		_T("килограмм"), _T("конференц"), _T("креп"), _T("кросс"), _T("лейб"),
		_T("лже"), _T("мини"), _T("макси"), _T("ново"),	_T("норд"), _T("обер"),
		_T("ост"), _T("патч"), _T("полу"), _T("премьер"), _T("пресс"), _T("радио"),
		_T("риск"), _T("сан"), _T("санта"), _T("санкт"), _T("северо"), _T("сент"),
		_T("сигма"), _T("сити"),	_T("социал"), _T("спас"), _T("старо"), _T("стоп"),
		_T("теле"), _T("трейд"), _T("убер"), _T("ультра"), _T("усть"), _T("ура"),
		_T("фальш"), _T("шайтан"), _T("штаб"), _T("штабс"), _T("экс"), _T("эль"),
		_T("экспресс"), _T("южно") };


	//TODO: Н-Алтайск
	if (std::find(indeclinable.begin(), indeclinable.end(), parts[0]) != indeclinable.end()
		|| !ContainsVowel(parts.front())) /*Н-Алтайск*/ {
		if (parts.size() == 2) {
			return true;
		}

		return func(std::vector<std::tstring>(parts.begin(), parts.end() - 1));
	}

	// Чеченские Цоцин-Юрт, Закан-Юрт, Ножай-Юрт и др.
	if (parts.back() == _T("юрт")) {
		return true;
	}

	// Князе-Петровск, Пошехонье-Володарск
	if (parts.size() == 2
		&& (EndsWithOneOf(parts.front(), _T("о,е")) && !EndsWith(parts.front(), _T("ье")))
		&& EndsWith(parts.back(), _T("ск"))) {
		return true;
	}

	if (parts.size() == 2 
		&& HasAdjectivalEnding(parts.back().rbegin(), parts.back().rend())
		&& EndsWithOneOf(parts.front(), _T("о,е")) 
		&& !EndsWithOneOf(parts.front(), _T("ое,ье"))) {
		// Прилагательные типа "тёмно-зелёный, рабоче-крестьянский"
		return true;
	}

	return false;
}

