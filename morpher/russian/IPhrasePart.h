#pragma once
#include "../gc.h"
#include "../tstring.h"
#include "Global.h"

class IPhrasePart : public virtual GC::Object
{
public:

	virtual std::tstring GetFormByCase(Padeg padeg, bool animate) const = 0;

	virtual bool PaulcalIsSingular() const = 0;
};
