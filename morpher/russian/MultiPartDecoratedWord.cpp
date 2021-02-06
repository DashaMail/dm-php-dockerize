// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "MultiPartDecoratedWord.h"

MultiPartDecoratedWord::MultiPartDecoratedWord(
	const std::vector<DecoratedWord>& decoratedWords)
		: decoratedWords(decoratedWords),
		  lemma(DecoratedWord::Join(decoratedWords, _T("-")))
{
}

const std::tstring& MultiPartDecoratedWord::LastPart() const
{
	return decoratedWords[decoratedWords.size() - 1].Lemma();
}

const std::tstring& MultiPartDecoratedWord::FirstPart() const
{
	return decoratedWords[0].Lemma();
}

const std::vector<DecoratedWord>& MultiPartDecoratedWord::Parts() const
{
	return decoratedWords;
}

std::size_t MultiPartDecoratedWord::PartCount() const
{
	return decoratedWords.size();
}

std::tstring MultiPartDecoratedWord::Lemma() const
{
	return lemma;
}

std::tstring MultiPartDecoratedWord::DecoratedLemma() const
{
	std::vector<std::tstring> strings;

	for (auto word : decoratedWords)
	{
		strings.push_back(word.DecoratedLemma());
	}

	std::tstring str;
	for (auto s : strings)
	{
		str += s.append(_T("-"));
	}

	str = str.substr(0, str.size() - 1);

	return str;
}

bool MultiPartDecoratedWord::IsSameCaps(const Caps& caps) const
{
	for (auto part = Parts().begin(); part != Parts().end(); ++part)
	{
		if (PartCount() > 1 && part->Lemma().length() < 3) continue; // Ортега-и-Гассет

		if (part->Lemma().compare(_T("оглы")) == 0) continue;
		if (part->Lemma().compare(_T("кызы")) == 0) continue;

		if (!part->GetCaps().IsSameType(caps)) return false;
	}

	return true;
}
