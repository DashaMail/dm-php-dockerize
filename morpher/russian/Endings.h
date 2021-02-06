#pragma once
#include "Global.h"
// набор падежных окончаний (парадигма склонения)
class Endings
{
protected:
	virtual ~Endings() {}

public:

	virtual std::tstring operator [] (Padeg c) const = 0;

	// винительный одушевленного слова равен родительному
	virtual bool AccAnimEqualsGen() const = 0;

	// несклоняемое
	virtual bool IsNotInflected() const = 0;

	virtual EndingsType GetEndingsType() const = 0;
};