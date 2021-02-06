#include <stdio.h>

#include "../russian/Declension.h"
#include "../russian/RussianCurrencySpeller.h"
#include <iostream>

using namespace Morpher::Russian;

int main ()
{
	setlocale(LC_CTYPE, "");
	// This is your main Declension object. Constructing it currently takes around 200 ms and this time may vary in future versions.
	// You may want to construct it once at application start up or on first use.  Try to reuse it when possible.  Keep out of loops.
	// It also holds on to quite a bit of memory (~10MB) so it is up to you how long you want to keep it in scope.
	// It is stateless and can always be recreated at the expense of time.
	Declension declension;
	std::unique_ptr<Parse> fio_split = declension.parse(_T("Филенко Сергей Сергеевич"), Attribute_Fio);
	std::tstring name = fio_split->fio()->GetName();
	std::tstring surname = fio_split->fio()->GetSurname();
	std::tstring patronymic = fio_split->fio()->GetPatronymic();
	
	if (name != _T("Сергей"))
	{
		std::wcerr << "Incorrect name (Sergey expected) but was '" << name << "'\n";
	}

	if (surname != _T("Филенко"))
	{
		std::wcerr << "Incorrect surname (Filenko expected) but was '" << surname << "'\n";
	}

	if (patronymic != _T("Сергеевич"))
	{
		std::wcerr << "Incorrect patronymic (Sergeevich expected) but was '" << patronymic << "'\n";
	}

	std::unique_ptr<Parse> parse = declension.parse (_T("АТОМ"));
	if (parse->fio())
	{
		std::wcerr << "Fio should be NULL\n";
	}

	std::tstring l = declension.parse (_T("Ростов-на-Дону"))->locative (); 
	if (l != _T("в Ростове-на-Дону"))
	{
		puts ("Incorrect locative (v Rostove-na-Donu expected).");
		std::wcout << l << std::endl;
	}
	std::wcout << l << std::endl;

	if (parse->dative () != _T("АТОМУ"))
	{
		puts ("Incorrect dative.");
	}
	if (parse->locative () != _T("В АТОМЕ"))
	{
		puts ("Incorrect locative.");
	}
	if (declension.parse (_T("луг"))->locative () != _T("на лугу"))
	{
		puts ("Incorrect locative (2).");
	}
	if (declension.parse (_T("Нью-Йорк"))->accusative () != _T("Нью-Йорк"))
	{
		puts ("'New York' failed.");
	}
	if (declension.parse (_T("Бечвая Николози Омехиевич"))->accusative () != _T("Бечвая Николози Омехиевича"))
	{
		puts ("'Bechvaya' failed.");
	}
	if (declension.parse (_T("Стамбул"))->accusative () != _T("Стамбул")) puts ("'Stambul' failed.");
	if (declension.parse (_T("роялти"))->genitive () != _T("роялти")) puts ("'royalty' failed.");
	if (declension.parse (_T("Далянь"))->genitive () != _T("Даляня")) puts ("'Dalian' failed.");
	if (parse->plural ()->nominative () != _T("АТОМЫ"))
	{
		puts ("Incorrect plural nominative.");
	}
	if (parse->gender () != MASCULINE)
	{
		puts ("Incorrect gender.");
	}
	if (declension.parse (_T("ЧЕРНЫШЕВА ЮЛИЯ"))->gender () != FEMININE)
	{
		puts ("Incorrect gender (All Caps).");
	}
	if (declension.parse (_T("ЧЕРНЫШЕВА ЮЛИЯ ВЛАДИМИРОВА"))->gender () != FEMININE)
	{
		puts ("Incorrect gender (All Caps) with patronymic.");
	}
	if (declension.parse (_T("NON-RUSSIAN")))
	{
		puts ("parse() should return NULL for non-Russian input.");
	}
	if (declension.parse (_T("Нью-Й")) != nullptr)
	{
		puts ("parse() should return NULL for New-Y.");
	}
	if (declension.parse (_T("ночь-ь")) != nullptr)
	{
		puts ("parse() should return NULL for Noch-b.");
	}

	RussianCurrencySpeller speller(declension.get_declensor());

	auto result = speller.Spell(123, _T("BYN"), 0);

	return 0;
}

