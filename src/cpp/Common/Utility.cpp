#include "Common.h"
#include "Utility.h"

Tabs::Tabs(int _tabLevel) : mTabLevel(_tabLevel)
{
}

Tabs& Tabs::operator++()
{
	++mTabLevel;
	return *this;
}

Tabs& Tabs::operator--()
{
	--mTabLevel;
	return *this;
}

Tabs Tabs::Next() const
{
	return Tabs(mTabLevel + 1);
}

size_t Tabs::GetTabLevel() const
{
	return mTabLevel;
}

std::ostream& operator<<(std::ostream& _os, Tabs _tabs)
{
	size_t tabLevel = _tabs.GetTabLevel();
	while (tabLevel--)
		_os.put('\t');
	return _os;
}

std::string EscapeChar(char _c)
{
	switch (_c)
	{
		case '\\': return "\\\\";
		case '\n': return "\\n";
		case '\r': return "\\r";
		case '\t': return "\\t";
		case '\'': return "\\\'";
		case '\"': return "\\\"";
		default: return std::string(1, _c);
	}
}
