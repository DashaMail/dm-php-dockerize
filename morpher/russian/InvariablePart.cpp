// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "InvariablePart.h"

InvariablePart::InvariablePart(const std::tstring& s): s(s)
{
}

std::tstring InvariablePart::GetFormByCase(Padeg padeg, bool animate) const
{
	return s;
}

bool InvariablePart::PaulcalIsSingular() const
{
	return false;
}
