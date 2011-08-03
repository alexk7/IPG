#include "Common.h"
#include "Utility.h"

EscapeChar::EscapeChar(char _c) : c(_c)
{
}

std::ostream& operator<<(std::ostream& _os, EscapeChar _e)
{
	char c = _e.c;
	switch (c)
	{
		case '\n': c = 'n'; break;
		case '\r': c = 'r'; break;
		case '\t': c = 't'; break;
		
		case '\\':
		case '\'':
		case '\"':
			break;
			
		default:
			_os.put(c);
			return _os;
	}
	
	_os.put('\\');
	_os.put(c);
	return _os;
}
