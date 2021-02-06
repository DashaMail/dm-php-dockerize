#pragma once
#include <list>
#include <vector>
#include <functional>

#include "../Tokenizer.h"

class Common
{
public:
	static const std::tstring vowels;

	static bool IsVowel(TCHAR c);

	static bool ContainsVowel(const std::tstring & s);

	/// <summary>
	/// Включая Й, исключая Ъ и Ь.
	/// </summary>
	static bool IsConsonant(TCHAR c);

	static bool HasFamilyNameSuffix(const std::tstring & lowerCaseWord);

	static bool HasPronomialFamilyNameSuffix(const std::tstring & lowerCaseWord);

	static bool EndsWith(
		std::tstring::const_reverse_iterator si,
		std::tstring::const_reverse_iterator s_rend,
		std::tstring::const_reverse_iterator ei,
		std::tstring::const_reverse_iterator e_rend);

	static bool EndsWith(const std::tstring & s, const std::tstring & ending);


	static bool EndsWith(const std::tstring & lemma, TCHAR c);

	static bool EndsWithOneOf(
		std::tstring::const_reverse_iterator s,
		std::tstring::const_reverse_iterator rend,
		const std::tstring & endings);

	static bool EndsWithOneOf(
		const std::tstring & s,
		const std::tstring & endings);



	static bool HasAdjectivalEnding(
		std::tstring::const_reverse_iterator s,
		std::tstring::const_reverse_iterator send);

	static bool HasParticipalSuffix(
		std::tstring::const_reverse_iterator s,
		std::tstring::const_reverse_iterator send);

	// Определение того, что лемма имеет заданное окончание
	// с учётом особенностей орфографии.  Так, слово "баня"
	// признаётся имеющим окончание "а".
	// Также проверяет, что в основе достаточно букв.
	// C#: Common.HasEndings.
	static bool HasEnding(
		const std::tstring & lemma,
		std::tstring::const_reverse_iterator eb,
		std::tstring::const_reverse_iterator ee);

	static bool HasEnding(const std::tstring & lemma, const std::tstring & ending);

	static bool HasEndings(const std::tstring & lemma, const std::tstring & endings);

	static bool InflectedOnlyLastPart(
		const std::vector<std::tstring> & parts,
		std::function<bool(const std::vector<std::tstring>&)> func);

	static bool HasEnding2(
		const std::tstring & word,
		std::tstring::const_reverse_iterator eb,
		std::tstring::const_reverse_iterator ee);

	static bool Participle(
		std::tstring::const_reverse_iterator rbegin,
		std::tstring::const_reverse_iterator rend);

	static bool Participle(const std::tstring& s);

	/// <summary> Проверяет, является ли данное слово возвратным причастием. </summary>
	/// <param name="слово"> Слово в именительном падеже маленькими буквами. </param>
	static bool ReflexiveParticiple(const std::tstring& lowerCaseWord);
};