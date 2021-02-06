// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "TokenizedString.h"

TokenizedString::TokenizedString(
	const std::list<MultiPartDecoratedWord>& words,
	const std::list<std::tstring>& delims) : words(words), delims(delims)
{
}

std::list<MultiPartDecoratedWord>& TokenizedString::Words()
{
	return words;
}

std::list<std::tstring>& TokenizedString::Delims()
{
	return delims;
}
