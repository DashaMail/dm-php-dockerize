// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "ParameterlessParadigm.h"

ParameterlessParadigm::ParameterlessParadigm()
{
}

ParameterlessParadigm::ParameterlessParadigm(Paradigms::Row* row): row(row)
{
}

std::tstring ParameterlessParadigm::operator[](Padeg c) const
{
	return row->endings[c];
}

bool ParameterlessParadigm::AccAnimEqualsGen() const
{
	return row->accAnimEqualsGen;
}

bool ParameterlessParadigm::IsNotInflected() const
{
	return row == &Paradigms::Uninflected;
}

EndingsType ParameterlessParadigm::GetEndingsType() const
{
	return row->endingsType;
}
