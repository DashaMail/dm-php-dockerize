// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "Fio.h"
std::tstring FIO::GetName() const
{
	return this->Gen("I");
}

std::tstring FIO::GetSurname() const
{
	return this->Gen("F");
}

std::tstring FIO::GetPatronymic() const
{
	return this->Gen("O");
}

std::tstring FIO::Gen(const std::string & s) const
{
	std::tstring str;

	int i = 0;

	for (auto component : _components)
	{
		if (s.find(_fioCombination.at(i)) != std::tstring::npos)
		{
			if (str.length() > 0)
			{
				str.append(_T(" "));
			}

			str.append(component.DecoratedLemma());
		}

		++i;
	}

	return str;
}
