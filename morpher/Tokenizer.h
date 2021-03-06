﻿#pragma once

#include <string>
#include <algorithm>
#include <istream>

#include "tchar.h"

#include "tstring.h"


// character classification table, defined in Tokenizer.AlphaNum.cpp
extern char isAlphaNumeric [];
extern size_t isAlphaNumeric_length;

class Caps
{
public:
	static Caps GetCaps (const std::tstring & s)
	{
		if (s.empty ()) throw std::exception ();

		ApplyFunction a = GetApplyFunction (s);

		return Caps (a, s);
	}

	void Apply (std::tstring & s) const
	{
		apply (s, original);
	}

	std::tstring GetOriginal() const {
		return original;
	}

private:

	typedef void (*ApplyFunction) (std::tstring & s, const std::tstring & original);

	static ApplyFunction GetApplyFunction (const std::tstring & s)
	{
        if (IsUpper (s [0])) 
		{
            if (IsUpper (++s.begin (), s.end ())) return ApplyAllCaps;
            if (IsLower (++s.begin (), s.end ())) return ApplyFirstCap;
        } 

        if (IsLower (s.begin (), s.end ())) return ApplyAllLower;
        return ApplyFancyCaps;
	}

	Caps (ApplyFunction a, const std::tstring & original) 
		: apply (a)
		, original (original)
	{
	}

	ApplyFunction apply;
	std::tstring original;

	static void ApplyAllCaps (std::tstring & s, const std::tstring & original)
	{
		ToUpper (s.begin (), s.end ());
	}

	static void ApplyAllLower (std::tstring & s, const std::tstring & original)
	{
	}

	static void ApplyFirstCap (std::tstring & s, const std::tstring & original)
	{
		ToUpper (s.begin (), ++s.begin ());
	}

	static void ApplyFancyCaps (std::tstring & s, const std::tstring & original)
	{
		size_t len = (std::min)(s.length (), original.length ());

		for (size_t i = 0; i < len; ++i)
		{
			if (IsUpper (original [i]))
			{
				s [i] = ToUpper (s [i]);
			}
		}
	}

public:

	static Caps FirstCap;
	static Caps AllCaps;
	static Caps AllLower;

	bool IsFirstCap () const
	{
		return this->apply == ApplyFirstCap;
	}

	bool IsAllCaps () const
	{
		return this->apply == ApplyAllCaps;
	}

	bool IsAllLower () const
	{
		return this->apply == ApplyAllLower;
	}

	bool IsSameType (const Caps & that) const
	{
		return this->apply == that.apply;
	}

	static std::tstring lower;
	static std::tstring upper;

	static bool IsApostrophe (TCHAR c)
	{
		switch (c)
		{
		case _T('\''):
#ifdef UNICODE
		case _T('’'):
		case _T('”'):
		case _T('"'):
#endif
			return true;
		default:
			return false;
		}
	}

	static bool IsLetter (TCHAR c)
	{
		if (IsApostrophe (c)) return true; // п'ять, сем'я

		size_t i = 
#ifdef _UNICODE
			c;
#else
			(unsigned char) c;
#endif

		if (i >= isAlphaNumeric_length) return true;

		return isAlphaNumeric [i] != 0;
	}

	static bool IsUpper (TCHAR c)
	{
		return upper.find_first_of (c) != std::tstring::npos;
	}

	static bool IsLower (TCHAR c)
	{
		return lower.find_first_of (c) != std::tstring::npos;
	}

	static TCHAR ToLower (TCHAR c)
	{
		return Convert (c, upper, lower);
	}

	static TCHAR ToUpper (TCHAR c)
	{
		return Convert (c, lower, upper);
	}

	static TCHAR Convert (TCHAR c, const std::tstring & from, const std::tstring & to)
	{
		size_t i = from.find_first_of (c);

		return (i == std::tstring::npos) ? c : to [i];
	}

	static void ToLower (std::tstring & s)
	{
		for (size_t i=0; i < s.size (); ++i)
		{
			s [i] = ToLower (s [i]);
		}
	}

	static void ToUpper (std::tstring & s)
	{
		ToUpper (s.begin (), s.end ());
	}

	static void ToUpper (std::tstring::iterator begin, std::tstring::iterator end)
	{
		for (std::tstring::iterator s = begin; s != end; ++s)
		{
			*s = ToUpper (*s);
		}
	}

	static bool IsUpper (std::tstring::const_iterator begin, std::tstring::const_iterator end)
	{
		return All (begin, end, IsUpper);
	}

	static bool IsLower (std::tstring::const_iterator begin, std::tstring::const_iterator end)
	{
		return All (begin, end, IsLower);
	}

	static bool All (std::tstring::const_iterator begin, std::tstring::const_iterator end, bool (*Pred) (TCHAR c))
	{
		return std::count_if (begin, end, Pred) == end - begin;
	}
};

class Tokenizer
{
public:

	Tokenizer (const std::tstring & s, const std::tstring & alphabet) : s (s), start (0), cyrillic (alphabet)
	{
	}

	const std::tstring s;

	size_t start;

	std::tstring GetNextToken ()
	{
        return GetNext (false);
    }

    std::tstring GetNextSeparator ()
    {
        return GetNext (true);
    }

	void PutBack (const std::tstring & token)
	{
		start -= token.length ();
	}

    void SkipNextSeparator ()
    {
        while (start < s.length () && (! IsLetterOrDigit (s [start]))) ++start; 
    }

private:

    // Цифры нужны, чтобы правильно склонялся, например, MP3-плеер.
	bool IsLetterOrDigit (TCHAR c)
	{
		return Caps::IsLetter (c);
	}

    std::tstring GetNext (bool separator)
    {
        size_t i=start; 

		while (i < s.size () && (separator ^ IsLetterOrDigit (s [i]))) ++i; 

        std::tstring token = s.substr (start, i-start);

        start = i;

        return token;
    }

    std::tstring cyrillic;

public:

	static std::tstring Russian, Ukrainian;

    static bool IsRussian (const std::tstring & lowerCaseWord)
    {
        for (std::tstring::const_iterator pc = lowerCaseWord.begin (); pc != lowerCaseWord.end (); ++pc)
        {
            if (Russian.find (*pc) == std::tstring::npos) return false;
        }

        return true;
    }

    static bool IsRussianAllowHyphen (const std::tstring & lowerCaseWord)
    {
        for (std::tstring::const_iterator pc = lowerCaseWord.begin (); pc != lowerCaseWord.end (); ++pc)
        {
            if (*pc == _T('-')) continue;
            if (Russian.find (*pc) == std::tstring::npos) return false;
        }

        return true;
    }

    /// <summary>
    /// Исправляет буквы, ошибочно набранные латиницей.
    /// Возвращает true, если слово может считаться русским или украинским.
    /// </summary>
	static bool ChangeLatinToCyrillic (std::tstring * word) 
	{
		std::tstring w = *word;

		for (size_t ci = 0; ci < word->length (); ++ci)
		{
			TCHAR c = w [ci];

			if (Caps::IsApostrophe (c)) continue;

			TCHAR lowercaseC = Caps::ToLower (c);

			if (Cyrillic.find (lowercaseC) == std::tstring::npos) 
			{
				const std::tstring lat = _T("CcAaOoEePpHTBKMiIXxy");
				const std::tstring cyr = _T("СсАаОоЕеРрНТВКМіІХху");
				size_t i = lat.find (c);
				if (i == std::tstring::npos) return false;
				w [ci] = cyr [i];
			}
		}

		*word = w;

		return true;
	}

	static std::tstring Cyrillic;
};

// defined in win1251.cpp
extern wchar_t win1251toUnicode [];

class LemmaReader
{
public:

    typedef const std::tstring (*Add) (const std::tstring & lemma, const std::tstring & prevLemma, size_t sameCount);

    static const std::tstring CompareFromBeginning (const std::tstring & lemma, const std::tstring & prevLemma, size_t sameCount)
    {
        return prevLemma.substr (0, sameCount) + lemma;
    }

    static const std::tstring CompareFromEnd (const std::tstring & lemma, const std::tstring & prevLemma, size_t sameCount)
    {
        return lemma + prevLemma.substr (prevLemma.length () - sameCount, sameCount);
    }

private:

	std::istream & file;
	std::tstring prevLemma;
	Add add;

public:

	LemmaReader (std::istream & file, Add add) : file (file), prevLemma (_T("")), add (add) {}

	std::istream & Stream ()
	{
		return file;
	}

	const std::tstring GetNext (unsigned char * b) 
	{
		// прочитать число букв леммы, 
		// совпадающих с предыдущей леммой
		size_t sameCount = (unsigned char) file.get ();

		if (file.eof ()) return _T("");

		// прочитать лемму
		std::tstring lemma = _T("");
		unsigned char c;
		for (;;) 
		{
			c = file.get ();
			c = ~c;
			if (c < 32) break;
			TCHAR wc = 
#ifdef _UNICODE
				(c < 128) ? c : win1251toUnicode [c-128];
#else
				c;
#endif
			lemma += wc;
		}

		lemma = add (lemma, prevLemma, sameCount);

		prevLemma = lemma;

#if DEBUG
		if (!Tokenizer::IsRussianAllowHyphen (lemma))
		{
			throw runtime_error ("asdf");
		}
#endif
		*b = c;

		return prevLemma;
	}
};
