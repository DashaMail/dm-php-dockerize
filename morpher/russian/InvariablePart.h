#pragma once
#include "IPhrasePart.h"

class InvariablePart : public IPhrasePart
{
	std::tstring s;

public:

	InvariablePart(const std::tstring& s);

	virtual std::tstring GetFormByCase(Padeg padeg, bool animate) const override;

	virtual bool PaulcalIsSingular() const override;
};