#pragma once
#include <cassert>
#include "IPhrasePart.h"
#include "InvariablePart.h"
#include <list>

class PhraseBuilder // C#: PhraseBuilder
{
	std::list <GC::Ptr <const IPhrasePart> > parts;

public:

	const std::list<GC::Ptr<const IPhrasePart>>& Parts() const;

	void Add(const std::tstring& s);

	void Add(GC::Ptr<const IPhrasePart> part);
};
