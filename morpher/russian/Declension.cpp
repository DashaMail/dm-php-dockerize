// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "Declension.h"
#include "morpher.h"

namespace Morpher
{
	namespace Russian
	{
		Declension::Declension ()
			: declensor (new RussianDeclensor ())
		{
		}

		Declension::~Declension ()
		{
			delete declensor;
		}

		std::unique_ptr<Parse> Declension::parse (const std::tstring & input, int attributes) const
		{
			Phrase * phrase = declensor->Analyse (
				input,
				static_cast<Attributes>(attributes));
			// ReSharper disable CppSmartPointerVsMakeFunction
			// make_unique входит в С++14, для совместимости GCC собирается с С++11 флагом
			return phrase ? std::unique_ptr<Parse>(new Parse(phrase)) : nullptr;
			// ReSharper restore CppSmartPointerVsMakeFunction
		}

		Parse::Parse (const Phrase * phrase) 
			: Paradigm (phrase)
			, _plural (phrase->Plural () ? new Paradigm (phrase->Plural ()) : 0)
			, _fio (phrase->Fio())
		{
		}

		Gender Parse::gender () const
		{
			switch (phrase->getRodChislo ())
			{
				case SingMasc: return MASCULINE;
				case SingFem : return FEMININE;
				case SingNeut: return NEUTER;
				case Plural:   return PLURAL;
				case RodChislo_Count: break;
			}

			throw "Bad RodChislo value.";
		}

		Parse::~Parse ()
		{
			if (_plural) delete _plural;
			if (_fio) { delete _fio; }
		}

		Paradigm::Paradigm (const Phrase * phrase)
			: phrase (phrase)
		{
		}

		Paradigm::~Paradigm ()
		{
			delete phrase;
		}

		std::tstring Paradigm::nominative     () const { return phrase->getImen  (); }
		std::tstring Paradigm::genitive       () const { return phrase->getRod   (); }
		std::tstring Paradigm::dative         () const { return phrase->getDat   (); }
		std::tstring Paradigm::accusative     () const { return phrase->getVin   (); }
		std::tstring Paradigm::instrumental   () const { return phrase->getTvor  (); }
		std::tstring Paradigm::prepositional  () const { return phrase->getPred  (); }
		std::tstring Paradigm::prepositionalO () const { return phrase->getPredO (); }
		std::tstring Paradigm::locative       () const { return phrase->getMest  (); }
	}
}
