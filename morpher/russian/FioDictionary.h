#pragma once
#include "Global.h"
#include <vector>
#include "../tstring.h"

class FioDictionary
{
	struct Entry
	{
		Entry(const std::tstring& lemma, FioPart fioPart);

		std::tstring Lemma;
		FioPart fioPart;
	};

	struct EntryComparer
	{
		bool operator ()(const Entry& ths, const Entry& that) const;
	};

	typedef std::vector <Entry> Entries;
	Entries entries;

public:

	FioDictionary();

	class CompareByLemmas
	{
	public:
		bool operator ()(const Entry& e1, const Entry& e2) const;
	};

	bool Contains(const std::tstring& lemma) const;

	bool Contains(const std::tstring& lemma, FioPart fioPart) const;
};
