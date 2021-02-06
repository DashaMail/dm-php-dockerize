#pragma once
#include "Info.h"
//#include "GramInfo.h"

class _Indirect : public Info // C#: _Косвенный
{
public:
	_Indirect();
	virtual const GramInfo* Nominative() const override;
};
