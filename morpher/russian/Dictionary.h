#pragma once
#include <list>
#include "Global.h"
#include "GramInfo.h"
#include "GramInfoFactory.h"
#include "../Tokenizer.h"
#include "Common.h"

class Dictionary
{
	// сравнение строк с конца
	class ReverseLess
	{
		class CharLess
		{
			unsigned char table[
#ifdef _UNICODE
				0x4FA // вмещает все кириллические буквы в Юникоде
#else
				256
#endif
			];

			size_t IndexOf(TCHAR c) const;

		public:

			CharLess();

			bool operator ()(TCHAR c1, TCHAR c2) const;

			static unsigned ToTableIndex(TCHAR c);
		};

	public:

		bool operator ()(const std::tstring& s1, const std::tstring& s2) const;
	};

	struct Entry
	{
		Entry(const std::tstring& lemma, const Info* gi);

		std::tstring Lemma;
		const Info * GramInfo;
	};

	typedef std::vector <Entry> Entries;
	Entries entries;

	GramInfoFactory gramInfoFactory;

	std::map <std::tstring, const GramInfo *> homonyms;

	class SubstPlural : public Endings {
	public:
		SubstPlural();

		SubstPlural(unsigned char nomEnding, unsigned char genEnding);

	private:
		unsigned char nomEnding;
		unsigned char genEnding;

		virtual std::tstring operator [](Padeg c) const override;
		virtual bool AccAnimEqualsGen() const override;
		virtual bool IsNotInflected() const override;
		virtual EndingsType GetEndingsType() const override;
	};

	SubstPlural pluralEndings[3][3];

	bool pluralEndingsInitialised;

	const Endings* DecodeGramInfoByte1(unsigned char b);

	std::vector <GramInfo> gis;

	static bool Less(const Entry& e1, const Entry& e2);

public:

	Dictionary();

	const GramInfoFactory& getGramInfoFactory() const;

	const GramInfo* FindHomonym(const std::tstring& lemma) const;

	typedef std::list <const GramInfo *> ResultSet;

	void Find(const std::tstring& lemma, bool first, ResultSet& result) const;

	bool ContainsFullLemma(const std::tstring& lemma) const;

	bool CanBeGenitive(const std::tstring& lemma) const;

	Gender GetGender(const std::tstring& lemma) const;

	const GramInfo* Uninflected(RodChislo rodChislo, bool isNoun) const;

private:

	GramInfo uninflected[RodChislo_Count][2];

	static bool CanBeGenitive(Entries::const_iterator entry);

	void AddResult(
		ResultSet& results,
		Entries::const_iterator result,
		const std::tstring& lemma) const;
};
