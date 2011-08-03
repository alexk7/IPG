#ifndef IPG_UTILITY_H_
#define IPG_UTILITY_H_

struct EscapeChar
{
	EscapeChar(char _c);
	char c;
};

std::ostream& operator<<(std::ostream& _os, EscapeChar _e);
	
#endif
