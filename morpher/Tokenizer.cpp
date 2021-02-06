#include "Tokenizer.h"

std::tstring Caps::lower (_T("абвгдеёжзийклмнопрстуфхцчшщъыьэюяіїєѓ"));
std::tstring Caps::upper (_T("АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯІЇЄЃ"));

Caps Caps::FirstCap = GetCaps (_T("Яя"));
Caps Caps::AllCaps  = GetCaps (_T("ЯЯ"));
Caps Caps::AllLower = GetCaps (_T("яя"));

std::tstring Tokenizer::Russian =   _T("абвгдеёжзийклмнопрстуфхцчшщъыьэюя");
std::tstring Tokenizer::Ukrainian = _T("абвгдежзийклмнопрстуфхцчшщьэюяіїєѓ");
std::tstring Tokenizer::Cyrillic =  _T("абвгдеёжзийклмнопрстуфхцчшщъыьэюяіїєѓ");

