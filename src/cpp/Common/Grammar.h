#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include "Expression.h"

struct DefValue : Expression
{
	DefValue() : isNode(false) {}
	bool isNode; //exists in the traversal interface
};

typedef std::map<std::string, DefValue> Defs;
typedef Defs::value_type Def;

class Grammar
{
public:
	void ComputeIsLeaf();

	Defs defs;
	
private:
	void ComputeIsLeaf(Expression* _pExpression, std::set<DefValue*>& _visited);
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
