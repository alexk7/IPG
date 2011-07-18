#ifndef COMMON_H_
#define COMMON_H_

#include <cstdio>
#include <cassert>
#include <limits>

#include <ostream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>

#include <algorithm>
#include <iterator>

#include <stdexcept>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

class Tabs
{
public:
	Tabs(size_t _tabLevel = 0) : mTabLevel(_tabLevel) {}
	Tabs& operator++() { ++mTabLevel; return *this; }
	Tabs& operator--() { --mTabLevel; return *this; }
	Tabs Next() const  { return Tabs(mTabLevel + 1); }
	size_t GetTabLevel() const { return mTabLevel; }
	
private:
	size_t mTabLevel;
};

inline std::ostream& operator<<(std::ostream& _os, Tabs _tabs)
{
	size_t tabLevel = _tabs.GetTabLevel();
	while (tabLevel--)
		_os.put('\t');
	return _os;
}

inline std::string EscapeChar(char _c)
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

#endif /* COMMON_H_ */
