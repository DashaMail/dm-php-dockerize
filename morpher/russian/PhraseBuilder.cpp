// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "PhraseBuilder.h"

const std::list<GC::Ptr<const IPhrasePart>>& PhraseBuilder::Parts() const
{
	return parts;
}

void PhraseBuilder::Add(const std::tstring& s)
{
	if (s.empty()) return;

	Add(new InvariablePart(s));
}

void PhraseBuilder::Add(GC::Ptr<const IPhrasePart> part)
{
	assert(part);

	parts.push_back(part);
}
