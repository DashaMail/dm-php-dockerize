#pragma once
#include <list>
#include <vector>
#include <ctime>
#include <set>
#include "IMorpher.h"
#include "Dictionary.h"
#include "FioDictionary.h"
#include "IParsedPart.h"

typedef std::tstring(Phrase::*PhraseMemFun) () const;

class RussianDeclensor : public IMorpher // C#: Склонятель
{
	Dictionary dictionary;
	FioDictionary fioDict;

	static void Tokenize(
		const std::tstring& s,
		std::list<DecoratedWord>& decoratedWords,
		std::list<std::tstring>& separators);

	static void RemoveNonRussian(
		std::list<DecoratedWord>& decoratedWords,
		std::list<std::tstring>& separators);

	static bool IsHyphen(const std::tstring& s);

	// C#: ОбъединитьСловаСДефисом
	static void JoinHypenatedWords(
		std::list<DecoratedWord>& decoratedWords,
		std::list<std::tstring>& separators,
		std::list<MultiPartDecoratedWord>& result);

	template <class T>
	static void Move(std::list<T>& to, std::list<T>& from);

	/// <summary>
	/// Перенести столько разделителей, сколько частей в слове.
	/// </summary>
	static void Move(
		std::list<std::tstring>& currentDelimList,
		std::list<std::tstring>& separators,
		const std::list<MultiPartDecoratedWord>& currentWordList);

	static std::tstring Trim(const std::tstring& s);

	static bool IsDash(const std::tstring& s);

	static bool IsInitial(const std::tstring& s);

	static bool IsInitial(const MultiPartDecoratedWord& multiPartDecoratedWord);

	static void HandleInitials(
		std::list<MultiPartDecoratedWord>& components,
		std::list<std::tstring>& separators);

	bool NotFoundInDictionaryOrInfectionIsAmbiguous(const std::tstring& component) const;

	const GramInfo* Uninflected(RodChislo rodChislo, bool isNoun) const;

	std::string ChooseCombination(
		const std::list<MultiPartDecoratedWord>& components,
		RodChislo* rodChisloResult,
		Attributes atts) const;

	static RodChislo ConvertGenderToRodChislo(Gender rodChislo);

	static bool Matches(Gender gender, RodChislo rodChislo);

	bool GenderMatchesRodChislo(
		const std::tstring& lemma,
		RodChislo rodChislo,
		FioPart fioPart) const;

	struct FioPartAndGender
	{
		FioPart fioPart;
		Gender gender;

		FioPartAndGender(FioPart fioPart, Gender gender);
	};

	static FioPartAndGender GetFioPartAndGenderNullable(const std::tstring& word);

	FioPartAndGender GetfioPartAndGender(const MultiPartDecoratedWord& word) const;

	/// <summary>
	/// Возвращает NULL, если <paramref name="lemma"/> попадает на 
	/// несклоняемое или plural/средний род.
	/// </summary>
	const SingularOrPluralGramInfo* FindInCommonDictionary(
		const std::tstring& lemma,
		RodChislo rodChislo,
		FioPart fioPart) const;

	const GramInfoFactory& gramInfoFactory() const;

	static RodChislo Opposite(RodChislo rodChislo);

	static FioPart GetFioPart(char c);

	class ParsedPartFIO : public IParsedPart
	{
		const MultiPartDecoratedWord multiPartDecoratedWord;
		std::vector <const SingularOrPluralGramInfo *> gramInfo;

	public:
		ParsedPartFIO(
			const MultiPartDecoratedWord& multiPartDecoratedWord,
			const std::vector<const SingularOrPluralGramInfo *>& gramInfo
		);

		virtual bool AppendTo(
			PhraseBuilder& phraseBuilder,
			std::list<std::tstring>::const_iterator& separator,
			bool plural) const override;

		virtual GC::Ptr<IParsedPart> GetPlural() const;
	};

	void AnalyseFio(
		std::list<MultiPartDecoratedWord>& components,
		std::list<std::tstring>& separators,
		Attributes attributes,
		RodChislo* rodChislo,
		bool* isAnimate,
		bool* prepositionNA,
		std::string& fioCombination,
		std::list<GC::Ptr<IParsedPart>>& result) const;

	void ParseWord(
		const MultiPartDecoratedWord& word,
		RodChislo rodChislo,
		FioPart fioPart,
		std::vector<const SingularOrPluralGramInfo *>& gi) const;

	const SingularOrPluralGramInfo* ParseWord(
		const std::tstring& lemma,
		RodChislo rodChislo,
		FioPart fioPart) const;

	static Paradigms::Row* GetEndingsAuto(
		const std::tstring& word,
		bool masculine,
		FioPart fioPart);

	static Paradigms::Row* GetFamilyNameEndings(
		const std::tstring& familyName,
		bool masculine);

	static Paradigms::Row* GetGivenNameEndings(
		const std::tstring& givenName,
		bool masculine);

	/// <summary>
	/// Возвращает 0, если <paramref name="lemma"/> попадает на несклоняемое 
	/// или plural/средний род
	/// или если леммы нет в словаре ФИО и она имеет suffixes фамилии или отчества.
	/// </summary>
	const SingularOrPluralGramInfo* GetGramInfo(
		const std::tstring& lemma,
		FioPart fioPart,
		RodChislo rodChislo) const;

	bool OnlyLastPartIsInflected(const MultiPartDecoratedWord& word, FioPart fioPart) const;

	static bool IsSpecialWord(const MultiPartDecoratedWord& word);

	bool IsCommonNoun(const std::tstring& lemma) const;

	bool NotAHomonym(const std::tstring& lemma) const;

	bool IsCommonNoun(const std::list<MultiPartDecoratedWord>& components) const;

	bool IsCommonNounWithFamilyNameSuffix(const std::tstring& lemma) const;

	bool ContainsWordsFromFioDictionary(
		const std::list<MultiPartDecoratedWord>& components) const;

	static bool AllWordsLowercase(const std::list<MultiPartDecoratedWord>& components);

	static bool WordLikeIbn(const MultiPartDecoratedWord& word);

	static bool IsFirstCap(const MultiPartDecoratedWord& word);

	static bool AllWordsTitleCase(const std::list<MultiPartDecoratedWord>& components);

	static bool CaseSuggestsName(const std::list<MultiPartDecoratedWord>& components);

	static bool AllWordsInCaps(const std::list<MultiPartDecoratedWord>& components);

	static bool EndsInOvoEvo(const std::list<MultiPartDecoratedWord>& components);

	static bool ContainsPatronymic(const std::list<MultiPartDecoratedWord>& components);

	static bool DoesNotLookLikeFio(const std::list<MultiPartDecoratedWord>& components);

	bool HasFamilyNameSuffix(const std::tstring& lowerCaseWord) const;

	bool ContainsWordsWithFamilyNameSuffix(
		const std::list<MultiPartDecoratedWord>& components) const;

	bool IsOnlyOneWordWithFamilyNameSuffix(
		const std::list<MultiPartDecoratedWord>& components) const;

	bool LooksLikeFio(const std::list<MultiPartDecoratedWord>& components) const;

	class Word : public IParsedPart
	{
	public:
		virtual RodChislo GetRodChislo() const = 0;

		virtual bool IsPossessiveAdjective() const = 0;

		virtual bool IsNonAdjectivalNoun() const = 0;

		virtual bool IsAnimate() const = 0;

		virtual bool UseNA() const = 0;

		virtual std::tstring Lemma() const = 0;
	};

	class WordWithHyphens : public Word {
		const MultiPartDecoratedWord word;
		const std::vector<GC::Ptr<Word>> parts;

	public:
		WordWithHyphens(
			const MultiPartDecoratedWord& word,
			const std::vector<GC::Ptr<Word>>& parts);

		virtual bool IsNonAdjectivalNoun() const override;

		virtual bool IsPossessiveAdjective() const override;

		virtual RodChislo GetRodChislo() const override;

		virtual bool IsAnimate() const override;

		virtual bool UseNA() const override;

		virtual std::tstring Lemma() const override;

		virtual bool AppendTo(PhraseBuilder& phraseBuilder,
		                      std::list<std::tstring>::const_iterator& c,
		                      bool plural) const override;;
	};

	// Класс, реализующий преобразования основы 
	// при добавлении окончания (беглость гласной,
	// чередования согласных).
	// Аналог класса ОсноваИмени в .NET версии
	class WordMorpher {
		std::tstring stem, lemma;
		std::tstring lemmaEnding;
		bool endingStressed;
		bool volatileVowel;
		size_t lemmaEndingLength;

		enum StemType { Firm, Soft, Mixed };
		StemType stemType;

	public:
		WordMorpher(const std::tstring& lemma, size_t endingLength,
		            bool endingStressed, bool volatileVowel);

		// слова, имеющие особое окончание тв.п. мн.ч. -ЬМИ
		static bool IsAmiException(const std::tstring& stem);

		std::tstring AddEnding(std::tstring ending) const;

	private:

		bool FormHasFleetingVowel(const std::tstring& ending) const;

		static bool BeginsWithVowel(const std::tstring& s);

		static bool trial_expired();

		static void RemoveFleetingVowel(std::tstring& stem);

		static bool Slogovoe(const std::tstring& ending);

		static bool IsSoftening(TCHAR c);

		static bool StartsWithSoftening(const std::tstring& s);

		static bool IsConsonant(TCHAR c);

		static bool IsVowel(TCHAR c);

		static bool EndsWithConsonant(const std::tstring& s);

		static bool IsSibilantOrTse(TCHAR c);

		static bool EndsWithSibilantOrTse(const std::tstring& s);

		static bool ContainsVowel(const std::tstring& s);

		static bool IsGuttural(TCHAR c);
	};

	class SingleWordWithGramInfo : public IPhrasePart
	{
		const SingularOrPluralGramInfo * gi;
		DecoratedWord word;

		static std::tstring GetForm2(
			const SingularOrPluralGramInfo* gramInfo,
			const std::tstring& lemma,
			Padeg padeg,
			bool isAnimate);

	public:

		SingleWordWithGramInfo(
			const SingularOrPluralGramInfo* gi,
			const DecoratedWord& word);

		virtual std::tstring GetFormByCase(Padeg padeg, bool animate) const override;

		virtual bool PaulcalIsSingular() const override;
	};

	class DictionaryWord : public Word // C# СловарноеСлово
	{
		const MultiPartDecoratedWord word;
		const GramInfo * const gi;

	public:
		DictionaryWord(const MultiPartDecoratedWord& word, const GramInfo* gi);

		// Пока только для отладки.
		virtual std::tstring Lemma() const override;

		virtual RodChislo GetRodChislo() const override;

		virtual bool IsPossessiveAdjective() const override;

		virtual bool IsNonAdjectivalNoun() const override;

		virtual bool IsAnimate() const override;

		virtual bool UseNA() const override;

		virtual bool AppendTo(
			PhraseBuilder& phraseBuilder,
			std::list<std::tstring>::const_iterator& separator,
			bool plural) const override;

		std::tstring GetPluralLemma() const;

		static std::tstring ApplyAlterations(std::tstring singularLemma);
	};

	static bool ContainsVowel(const std::tstring& s);

	/// <summary>
	/// Определить тип склонения простого слова (слова без дефисов)
	/// в соответствии с основным словарём и словарём особых слов.
	/// </summary>
	void FindDeclensionForSimpleWord(
		const MultiPartDecoratedWord& word,
		bool first,
		std::list<GC::Ptr<Word>>& result) const;

	// TODO: Перенести куда-то
	class GramInfoGenders {
	private:
		GramInfoFactory gramInfoFactory;
		GramInfo gramInfoFem;
		GramInfo gramInfoMasc;
		GramInfo gramInfoNeut;
		GramInfo gramInfoPlural;
	public:
		GramInfoGenders();

		const GramInfo* GramInfoFem() const;

		const GramInfo* GramInfoMasc() const;

		const GramInfo* GramInfoNeut() const;

		const GramInfo* GramInfoPlural() const;
	};

	GramInfoGenders IndeclinableGramInfoGlobal;

	GC::Ptr<DictionaryWord> Indeclinable(
		RodChislo rodChiso,
		const MultiPartDecoratedWord& word) const;

	void AddIndeclinable(
		const MultiPartDecoratedWord& word,
		std::list<GC::Ptr<Word>>& result,
		RodChislo rodChislo) const;

	void IndeclinableWithoutGender(
		const MultiPartDecoratedWord& word,
		std::list<GC::Ptr<Word>>& result) const;

	class SpecialDictionary // C#: СловарьОсобыхСлов
	{
		class SpecialWord : public GC::Object // C#: ОсобоеСлово
		{
			const std::vector <std::tstring> singularForms;

			GC::Ptr<const SpecialWord> plural; //GC::Ptrиспользуется для упрощения управления памятью - не нужен деструктор, operator = и конструктор копии

			const RodChislo rodChislo;
			const bool IsNoun;

			/// <summary>
			/// false означает, что либо это неодушевлённое существительное,
			/// либо категория одушевлённости неприменима к этому слову
			/// (если это прилагательное или местоимение).
			/// </summary>
			const bool isAnimate;

		public:

			SpecialWord(
				RodChislo rodChislo,
				bool isAnimate,
				bool IsNoun,
				const std::vector<std::tstring>& singularForms,
				const std::vector<std::tstring>& pluralForms);

			const std::tstring& Lemma() const;

			bool IsNonAdjectivalNoun() const;

			RodChislo GetRodChislo() const;

			bool IsAnimate() const;

			const SpecialWord* GetPlural() const;

			std::tstring GetFormByCase(Padeg padeg, bool anim) const;

		private:

			std::tstring GetFormByCaseInternal(Padeg padeg, bool anim) const;
		};

		class DecoratedSpecialWord : public Word, public IPhrasePart
		{
			const MultiPartDecoratedWord word;
			const SpecialWord & specialWord;

		public:
			DecoratedSpecialWord(
				MultiPartDecoratedWord word,
				const SpecialWord& specialWord);

			virtual bool IsNonAdjectivalNoun() const override;

			virtual RodChislo GetRodChislo() const override;

			virtual bool IsAnimate() const override;

			virtual bool IsPossessiveAdjective() const override;

			virtual bool AppendTo(
				PhraseBuilder& phraseBuilder,
				std::list<std::tstring>::const_iterator& separator,
				bool plural) const override;

			DecoratedSpecialWord* GetPlural() const;

			// IPhrasePart.GetFormByCase 
			virtual std::tstring GetFormByCase(Padeg padeg, bool isAnimate) const override;

			// IPhrasePart.PaulcalIsSingular
			virtual bool PaulcalIsSingular() const override;

			virtual bool UseNA() const override;

			virtual std::tstring Lemma() const override;
		};

		class Less
		{
		public:
			bool operator ()(const SpecialWord& w1, const SpecialWord& w2) const;
		};

		typedef std::multiset<SpecialWord, Less> Words;

		Words words;

	public:

		SpecialDictionary();

	private:

		static std::vector<std::tstring> ReadForms(LemmaReader& lemmaReader);

		void Add(
			bool IsNoun,
			RodChislo rodChislo,
			bool anim,
			std::vector<std::tstring> singularForms,
			const std::vector<std::tstring>& pluralForms);

		static std::vector<std::tstring> RemoveYo(const std::vector<std::tstring>& forms);

		static std::tstring ReplaceYo(std::tstring s);

	public:

		void Find(
			const MultiPartDecoratedWord& word,
			std::list<GC::Ptr<Word>>& result) const;
	};

	SpecialDictionary specialDictionary;

	class WordLikeRostovNaDonu : public Word
	{
	public:
		WordLikeRostovNaDonu(
			const MultiPartDecoratedWord& lemma,
			const GC::Ptr<Word>& firstPart);

		bool IsNonAdjectivalNoun() const override;
		RodChislo GetRodChislo() const override;
		bool IsAnimate() const override;
		bool IsPossessiveAdjective() const override;
		bool AppendTo(
			PhraseBuilder & phraseBuilder, 
			std::list <std::tstring>::const_iterator & separator,
			bool plural) const override;
		bool UseNA() const override;
		std::tstring Lemma() const override;

	private:
		MultiPartDecoratedWord _lemma;
		std::tstring _sLemma;
		GC::Ptr<Word> _firstPart;

	};

	static bool NeedToFindDeclensionForFirstWordOnly(const MultiPartDecoratedWord& word);


	template<typename T, typename R>
	static std::vector<R> VectorOf(const std::vector<T> vec, std::function<R(T)> func);

	static GC::Ptr<Word> ChooseWord(
		RodChislo rodChislo,
		const std::list<GC::Ptr<Word>>& words);

	static GC::Ptr<Word> FindWordOfTheSpecifiedNumber(
		const std::list<GC::Ptr<Word>>& words,
		bool plural);

	void AnalyzeHyphenatedWord(
		const MultiPartDecoratedWord& word,
		bool first,
		std::list<GC::Ptr<Word>>& result) const;

	void FindDeclension(
		const MultiPartDecoratedWord& word,
		bool first,
		std::list<GC::Ptr<Word>>& result) const;

	static unsigned GetUniqueAttributes(const std::list<GC::Ptr<Word>>& words);

	static bool IsStopWord(const MultiPartDecoratedWord& word);

	bool CanBeGenitive(const MultiPartDecoratedWord& word) const;

	void AnalyseNaric(
		std::list<MultiPartDecoratedWord>& components,
		std::list<std::tstring>& separators,
		Attributes flags,
		RodChislo* rodChislo,
		bool* isAnimate,
		bool* prepositionNA,
		std::string& fioCombination,
		std::list<GC::Ptr<IParsedPart>>& result) const;

	static RodChislo GetRodChislo(
		unsigned attributes,
		const std::list<GC::Ptr<Word>>& matches);

	static void SecondPass(
		const std::list<std::list<GC::Ptr<Word>>>& matches,
		const std::list<MultiPartDecoratedWord>& components,
		const std::list<std::tstring>& separators,
		Attributes flags,
		RodChislo rodChislo,
		bool* isAnimate,
		bool* prepositionNA,
		std::list<GC::Ptr<IParsedPart>>& words);

	static Word* GetWordThatMatches(
		const std::list<GC::Ptr<Word>>& words,
		RodChislo rodChislo);

	typedef void (RussianDeclensor::*AnalysisMethod) (
		std::list <MultiPartDecoratedWord> & components,
		std::list <std::tstring> & separators,
		Attributes attributes,
		RodChislo * rodChislo,
		bool * isAnimate,
		bool * prepositionNA,
		std::string & fioCombination,
		std::list <GC::Ptr<IParsedPart> > & result) const;

	AnalysisMethod ChooseParser(
		const std::list<MultiPartDecoratedWord>& components,
		Attributes attributes) const;

	static bool Flatten(
		PhraseBuilder& phraseBuilder,
		const std::list<GC::Ptr<IParsedPart>>& parsedComponents,
		const std::list<MultiPartDecoratedWord>& components,
		const std::list<std::tstring>& separators,
		bool plural);

public:

	RussianDeclensor();

	virtual ~RussianDeclensor();

	// Вызывающий должен сделать delete возвращенному значению,
	// а также свойству Phrase.Plural, если оно не NULL.
	virtual Phrase* Analyse(const std::tstring& s, Attributes attributes) const override;

	static Phrase* Indeclinable(const std::tstring& s, RodChislo rodChislo);

	static std::tstring BuildStringFromWordsAndSep(
		size_t startIndex,
		const std::list<DecoratedWord>& words,
		const std::list<std::tstring>& separators);

	static void MoveRightPartToleftSeparator(
		size_t startIndex,
		std::list<DecoratedWord>& words,
		std::list<std::tstring>& separators);

	Phrase* MoveIndeclinable(
		std::list<DecoratedWord>& words,
		std::list<std::tstring>& separators) const;

	Phrase* AnalyseInternal(const std::tstring& s, Attributes attributes) const;
};
