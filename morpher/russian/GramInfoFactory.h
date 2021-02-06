#pragma once
#include "ParameterlessParadigm.h"
#include "SingularOrPluralGramInfo.h"
#include "Paradigms.h"
#include <vector>

class GramInfoFactory
{
	ParameterlessParadigm masculineFamilyNameEndings;
	SingularOrPluralGramInfo masculineFamilyName;
	std::vector <ParameterlessParadigm> singularEndings;

	void InitEndings();

	SingularOrPluralGramInfo gis[sizeof(Paradigms::table) / sizeof(*Paradigms::table)][2];

	void InitGramInfos();

	static int IndexOf(Paradigms::Row* row);

public:

	GramInfoFactory();

	const SingularOrPluralGramInfo* MasculineFamilyName() const;

	const SingularOrPluralGramInfo* GetGramInfo(
		Paradigms::Row* row,
		bool endingStressed) const;

	const std::vector<ParameterlessParadigm>& GetEndings() const;
};
