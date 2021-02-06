#pragma once
#include <list>
#include "MultiPartDecoratedWord.h"

class FIO
{
public:
	FIO(
		const std::list<MultiPartDecoratedWord> & components,
		const std::string & fioCombination)
		: _components(components), _fioCombination(fioCombination) { }

	std::tstring GetName() const;
	std::tstring GetSurname() const;
	std::tstring GetPatronymic() const;

private:
	std::list<MultiPartDecoratedWord> _components;
	std::string _fioCombination;

	std::tstring Gen(const std::string & s) const;
};

