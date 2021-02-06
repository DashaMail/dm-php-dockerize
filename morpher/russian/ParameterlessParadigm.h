#pragma once
#include "Endings.h"
#include "Paradigms.h"

// парадигма без параметров,
// окончания выбираются из таблицы
class ParameterlessParadigm : public Endings
{
public:
	Paradigms::Row * row; // TODO make private
	ParameterlessParadigm(); // only for creating arrays, will be init'd with placement new.
	ParameterlessParadigm(Paradigms::Row* row);
	virtual std::tstring operator [](Padeg c) const override;
	virtual bool AccAnimEqualsGen() const override;
	virtual bool IsNotInflected() const override;
	virtual EndingsType GetEndingsType() const override;
};