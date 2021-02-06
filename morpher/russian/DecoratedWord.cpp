// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "DecoratedWord.h"

DecoratedWord::DecoratedWord(const std::tstring& lemma, Caps caps, bool yer, bool sya):
	lemma(lemma), caps(caps), yer(yer), sya(sya)
{
}

DecoratedWord DecoratedWord::Get(std::tstring word)
{
	Caps caps = Caps::GetCaps(word);

	Caps::ToLower(word);

	bool yer = word.length() > 1 && Common::EndsWith(word, _T("ъ"));

	if (yer) word = word.substr(0, word.length() - 1);

	// отщепление аффикса -СЯ у возвратных причастий
	bool sya = Common::ReflexiveParticiple(word);
	if (sya) word = word.substr(0, word.length() - 2);

	return DecoratedWord(word, caps, yer, sya);
}

bool DecoratedWord::YerAllowed(const std::tstring& form)
{
	return Common::IsConsonant(*form.rbegin());
}

std::tstring DecoratedWord::ApplyTo(const std::tstring& word) const
{
	std::tstring s = word;

	if (yer && YerAllowed(s)) s += _T('ъ');

	if (sya) s += _T("ся");

	caps.Apply(s);

	return s;
}

const std::tstring& DecoratedWord::Lemma() const
{
	return lemma;
}

const Caps& DecoratedWord::GetCaps() const
{
	return caps;
}

std::tstring DecoratedWord::DecoratedLemma() const
{
	return ApplyTo(lemma);
}

DecoratedWord DecoratedWord::Clone(const std::tstring& newLemma) const
{
	return DecoratedWord(newLemma, caps, yer, sya);
}

std::tstring DecoratedWord::Join(
	const std::vector<DecoratedWord>& v,
	const std::tstring& separator)
{
	if (v.empty()) return _T("");

	if (v.size() == 1) return (*v.begin()).Lemma();

	size_t total = 0;

	for (std::vector<DecoratedWord>::const_iterator s = v.begin(); s != v.end(); ++s)
	{
		total += s->Lemma().length() + separator.length();
	}

	if (total == 0) return _T("");

	std::tstring js;

	js.reserve(total - separator.length());

	for (std::vector<DecoratedWord>::const_iterator s = v.begin(); s != v.end(); ++s)
	{
		if (s != v.begin()) js += separator;

		js += s->Lemma();
	}

	return js;
}
