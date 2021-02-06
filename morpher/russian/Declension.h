#pragma once

#include "../tstring.h"
#include "Fio.h"
#include <memory>

class RussianDeclensor;
class Phrase;
class FIO;

namespace Morpher
{
	namespace Russian
	{
		class Paradigm
		{
		protected:
			const Phrase * phrase;

		public:
			explicit Paradigm (const Phrase * phrase);
			~Paradigm ();

			std::tstring nominative     () const; // e.g. Иван,     Марья,   лес,    луг,     тень
			std::tstring genitive       () const; // e.g. Ивана,    Марьи,   леса,   луга,    тени
			std::tstring dative         () const; // e.g. Ивану,    Марье,   лесу,   лугу,    тени
			std::tstring accusative     () const; // e.g. Ивана,    Марью,   лес,    луг,     тень
			std::tstring instrumental   () const; // e.g. Иваном,   Марьей,  лесом,  лугом,   тенью
			std::tstring prepositional  () const; // e.g. Иване,    Марье,   лесе,   луге,    тени
			std::tstring prepositionalO () const; // e.g. об Иване, о Марье, о лесе, о луге,  о тени
			std::tstring locative       () const; // e.g. в Иване,  в Марье, в лесу, на лугу, в тени

		private: // copy protection
			void operator = (const Paradigm &) const {}
			Paradigm (const Paradigm &): phrase(nullptr) {}
		};

		enum Gender
		{
			MASCULINE, FEMININE, NEUTER, PLURAL
		};

		class Parse : public Paradigm
		{
			const Paradigm * _plural;
			const FIO * _fio;

		public:
			explicit Parse (const Phrase * phrase);
			~Parse ();

			const Paradigm * plural () const {return _plural;}
			const FIO * fio() const { return _fio; }

			Gender gender () const;
		};

		class Declension
		{
			const RussianDeclensor * const declensor;

		private: // copy protection
			void operator = (const Declension &) const {}
			Declension (const Declension &) : declensor (nullptr) {}

		public:
			Declension (); // this can be time-consuming; don't put it in a loop
			~Declension ();

			std::unique_ptr<Parse> parse (const std::tstring & input, int attributes = 0) const;

			const RussianDeclensor * get_declensor () const {return declensor;} // временный костыль
		};
	}
}
