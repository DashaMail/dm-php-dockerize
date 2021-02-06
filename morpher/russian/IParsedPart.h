#pragma once
#include <list>
#include "../gc.h"
#include "PhraseBuilder.h"

class IParsedPart : public virtual GC::Object
{
public:
	virtual bool AppendTo(
		PhraseBuilder & phraseBuilder,
		std::list <std::tstring>::const_iterator &,
		bool plural) const = 0;
};