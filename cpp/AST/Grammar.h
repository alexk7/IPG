/*
 * Grammar.h
 *
 *  Created on: 17-08-2009
 *      Author: alexk7
 */

#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include "Expression.h"

struct DefValue : Expression
{
    DefValue() : isNode(false), isMemoized(false), isNodeRef(true) {}
    bool isNode;     //exists in the traversal interface
    bool isMemoized; //memoized to avoid backtracking in traversal
    bool isNodeRef;
};

typedef std::map<std::string, DefValue> Defs;
typedef Defs::value_type Def;

class Grammar
{
public:
	Defs defs;
};

Def& AddDef(Defs&, const std::string& _name, Expression&);
void Print(std::ostream&, const Defs&);
void Print(std::ostream&, const Grammar&);

inline std::ostream& operator<<(std::ostream& _os, const Grammar& _g)
{ 
	Print(_os, _g);
	return _os;
}

#endif /* GRAMMAR_H_ */
