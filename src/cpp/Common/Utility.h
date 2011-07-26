#ifndef IPG_UTILITY_H_
#define IPG_UTILITY_H_

class Tabs
{
public:
	Tabs(int _tabLevel = 0);
	Tabs& operator++();
	Tabs& operator--();
	Tabs Next() const;
	size_t GetTabLevel() const;
	
private:
	int mTabLevel;
};

std::ostream& operator<<(std::ostream& _os, Tabs _tabs);
std::string EscapeChar(char _c);

#endif
