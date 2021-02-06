#pragma once

#include "RussianNumberSpeller.h"

enum CurrencySpellErrorCode
{
	CurrencySpellErrorCode_Success,
	CurrencySpellErrorCode_NumberIsEmpty,
	CurrencySpellErrorCode_NumberIsNotANumber,
	CurrencySpellErrorCode_NumberIsTooBig,
	CurrencySpellErrorCode_NumberIsTooSmall,
	CurrencySpellErrorCode_DeclensionError,
	CurrencySpellErrorCode_RoundingNeeded,
};

const TCHAR * CurrencySpellErrorCodeToRussianTextMessage (CurrencySpellErrorCode errorCode);

struct CurrencySpellResult
{
	explicit CurrencySpellResult (const std::tstring & result) : result (result), errorCode (CurrencySpellErrorCode_Success) {}
	CurrencySpellResult (CurrencySpellErrorCode errorCode) : result (_T("")), errorCode (errorCode) {}

	std::tstring result;
	CurrencySpellErrorCode errorCode;
};

class RussianCurrencySpeller
{
	const class AbbrMorpher * abbrMorpher;

	const RussianNumberSpeller numberSpeller;

	CurrencySpellResult Spell2(const std::tstring& number, const std::tstring& unit, int padeg) const;

public:
	explicit RussianCurrencySpeller(const IMorpher* morpher);

	CurrencySpellResult Spell(double number, const std::tstring& unit, int padeg) const;

	CurrencySpellResult Spell(const std::tstring& number, const std::tstring& unit, int padeg) const;

	~RussianCurrencySpeller();
};
