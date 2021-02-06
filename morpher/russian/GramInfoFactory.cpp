// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "GramInfoFactory.h"
#include "Paradigms.h"

void GramInfoFactory::InitEndings()
{
	const size_t n = sizeof(Paradigms::table) / sizeof(*Paradigms::table);

	singularEndings.reserve(n);

	for (size_t i = 0; i < n; ++i)
	{
		if (Paradigms::table[i]->endings[Padeg_M] == nullptr)
		{
			Paradigms::table[i]->endings[Padeg_M] = Paradigms::table[i]->endings[Padeg_P];
		}

		singularEndings.push_back(ParameterlessParadigm(Paradigms::table[i]));
	}
}

void GramInfoFactory::InitGramInfos()
{
	size_t count = sizeof(gis) / sizeof(*gis);

	for (size_t i = 0; i < count; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			gis[i][j] = SingularOrPluralGramInfo(&singularEndings[i], false, j != 0);
		}
	}
}

int GramInfoFactory::IndexOf(Paradigms::Row* row)
{
	int count = sizeof(Paradigms::table) / sizeof(*Paradigms::table);
	for (int i = 0; i < count; ++i)
	{
		if (Paradigms::table[i] == row) 
			return i;
	}

	throw std::exception();
}

const SingularOrPluralGramInfo* GramInfoFactory::GetGramInfo(Paradigms::Row* row, bool endingStressed) const
{
	return &gis[IndexOf(row)][endingStressed ? 1 : 0];
}

GramInfoFactory::GramInfoFactory()
	: masculineFamilyNameEndings(&Paradigms::MasculineFamilyName),
      masculineFamilyName(&masculineFamilyNameEndings, false, false)
{
	InitEndings();

	InitGramInfos();
}

const SingularOrPluralGramInfo* GramInfoFactory::MasculineFamilyName() const
{
	return &masculineFamilyName;
}

const std::vector<ParameterlessParadigm>& GramInfoFactory::GetEndings() const
{
	return singularEndings;
}
