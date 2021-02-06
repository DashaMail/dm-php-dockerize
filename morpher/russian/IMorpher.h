#pragma once
#include "Phrase.h"

class IMorpher
{
public:
	virtual ~IMorpher() {}

	virtual Phrase * Analyse(const std::tstring & s, Attributes attributes) const = 0;
};