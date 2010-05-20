/*
 * Grammar.cpp
 *
 *  Created on: 17-10-2009
 *      Author: alexk7
 */

#include "../Common.h"
#include "Grammar.h"

static void Print(std::ostream& _os, const Grammar::Defs& _defs)
{
	Grammar::Defs::const_iterator i, iEnd = _defs.end();
	for (i = _defs.begin(); i != iEnd; ++i)
	{
		_os << i->first;
		_os << " <- ";
		_os << i->second;
		_os << std::endl;
	}
}

void Print(std::ostream& _os, const Grammar& _grammar)
{
    Print(_os, _grammar.defs);
    Print(_os, _grammar.subDefs);
}
