#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include "Expression.h"

struct DefValue : Expression
{
	DefValue() : isNode(false), isMemoized(false) {}
	bool isNode; //exists in the traversal interface
	bool isMemoized;
};

typedef std::map<std::string, DefValue> Defs;
typedef Defs::value_type Def;

class Grammar
{
public:
	void ComputeIsLeaf();
	void CreateSkipNodes();

	Defs defs;
	
private:
	void ComputeIsLeaf(Expression* _pExpression, std::set<DefValue*>& _visited);
	void CreateSkipNodes(Expression& _expr, const std::string& _name, int& _index);
	void CreateSkipNode(Expression& _refExpr, Expression& _skipExpr, const std::string& _name, int& _index);
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
