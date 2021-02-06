// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "FioDictionary.h"
#include <sstream>
#include "../Tokenizer.h"

FioDictionary::Entry::Entry(const std::tstring& lemma, FioPart fioPart): Lemma(lemma)
                                                                         , fioPart(fioPart)
{
}

bool FioDictionary::EntryComparer::operator()(const Entry& ths, const Entry& that) const
{
	if (ths.Lemma != that.Lemma) return ths.Lemma < that.Lemma;

	return ths.fioPart < that.fioPart;
}

FioDictionary::FioDictionary()
{
	std::stringstream file(std::string(fioDict, fioDict_size));

	file.exceptions(file.exceptions() | std::ios::eofbit);

	LemmaReader lemmaReader(file, LemmaReader::CompareFromBeginning);

	size_t count = Global::GetInt32(file);

	entries.reserve(count);

	for (size_t i = 0; i < count; ++i)
	{
		unsigned char b = 0;

		const std::tstring& lemma = lemmaReader.GetNext(&b);

		entries.push_back(Entry(lemma, static_cast<FioPart>(b)));
	}
}

bool FioDictionary::CompareByLemmas::operator()(const Entry& e1, const Entry& e2) const
{
	return e1.Lemma.compare(e2.Lemma) < 0;
}

bool FioDictionary::Contains(const std::tstring& lemma) const
{
	// Если здесь возникает Access violation, то скорее всего,
	// передан неправильный ключ в функцию Unlock.

	Entries::const_iterator entry = lower_bound(entries.begin(),
	                                            entries.end(),
	                                            Entry(lemma, static_cast<FioPart>(0)),
	                                            CompareByLemmas());

	if (entry == entries.end()) return false;

	return entry->Lemma == lemma;
}

bool FioDictionary::Contains(const std::tstring& lemma, FioPart fioPart) const
{
	Entries::const_iterator entry = lower_bound(entries.begin(),
	                                            entries.end(),
	                                            Entry(lemma, fioPart), EntryComparer());

	if (entry == entries.end()) return false;

	return entry->Lemma == lemma && entry->fioPart == fioPart;
}
