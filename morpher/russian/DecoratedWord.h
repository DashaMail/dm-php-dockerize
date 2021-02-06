#pragma once
#include "../Tokenizer.h"
#include "Common.h"

class DecoratedWord
{
	std::tstring lemma;

	Caps caps;

	bool yer; // Ъ

	bool sya; // ся

	DecoratedWord(const std::tstring& lemma, Caps caps, bool yer, bool sya);

public:

	static DecoratedWord Get(std::tstring word);

	static bool YerAllowed(const std::tstring& form);

	std::tstring ApplyTo(const std::tstring& word) const;

	const std::tstring& Lemma() const;

	const Caps& GetCaps() const;

	std::tstring DecoratedLemma() const;

	DecoratedWord Clone(const std::tstring& newLemma) const;

	static std::tstring Join(
		const std::vector<DecoratedWord>& v,
		const std::tstring& separator);
};