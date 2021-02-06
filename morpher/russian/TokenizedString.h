#pragma once
#include <list>
#include "MultiPartDecoratedWord.h"

class TokenizedString
{
	std::list <MultiPartDecoratedWord> words;
	std::list <std::tstring> delims;

public:
	TokenizedString(
		const std::list<MultiPartDecoratedWord>& words,
		const std::list<std::tstring>& delims);

	std::list<MultiPartDecoratedWord>& Words();

	std::list<std::tstring>& Delims();
};
