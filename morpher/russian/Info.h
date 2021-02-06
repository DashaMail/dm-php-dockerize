#pragma once

class GramInfo;

class Info
{
protected:
	virtual ~Info()
	{
	}

public:
	/// <summary>
	/// Возвращает NULL, если лемма этой статьи является словом в косвенном падеже.
	/// </summary>
	virtual const GramInfo * Nominative() const = 0; // C#: Именительный
};