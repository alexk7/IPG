/*
 * Grammar.h
 *
 *  Created on: 17-08-2009
 *      Author: alexk7
 */

#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include "Expression.h"

class Grammar
{
public:
	typedef std::map<std::string, Expression> Defs;
	Defs defs;
};

void Print(std::ostream& _os, const Grammar& _grammar);
inline std::ostream& operator<<(std::ostream& _os, const Grammar& _grammar) { Print(_os, _grammar); return _os; }

#endif /* GRAMMAR_H_ */
