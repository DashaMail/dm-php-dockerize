#pragma once
#include <vector>
#include "../tstring.h"
#include "DecoratedWord.h"

class MultiPartDecoratedWord
{
	std::vector <DecoratedWord> decoratedWords;
	std::tstring lemma;

public:

	MultiPartDecoratedWord(const std::vector<DecoratedWord>& decoratedWords);

	const std::tstring& LastPart() const;

	const std::tstring& FirstPart() const;

	const std::vector<DecoratedWord>& Parts() const;

	std::size_t PartCount() const;

	std::tstring Lemma() const;

	std::tstring DecoratedLemma() const;

	bool IsSameCaps(const Caps& caps) const;
};
