#include "Common.h"
#include "Grammar.h"

Def& AddDef(Defs& _defs, const std::string& _name, Expression& _e)
{
	std::pair<Defs::iterator, bool> result = _defs.insert(Def(_name, DefValue()));
	if (!result.second)
		throw std::runtime_error(str(boost::format("Duplicate definition: %1%") % _name));

	Def& newDef = *result.first;
	newDef.second.Swap(_e);
	return newDef;
}

void Print(std::ostream& _os, const Defs& _defs)
{
	Defs::const_iterator i, iEnd = _defs.end();
	for (i = _defs.begin(); i != iEnd; ++i)
	{
		_os << i->first << " <";
		if (i->second.isNode)
			_os << "=";
		else
			_os << "-";
		_os << " " << i->second;
		_os << std::endl;
	}
}

void Print(std::ostream& _os, const Grammar& _grammar)
{
	Print(_os, _grammar.defs);
}

void Grammar::ComputeIsLeaf()
{
	std::set<DefValue*> visited;

	Defs::iterator i, iEnd = defs.end();
	for (i = defs.begin(); i != iEnd; ++i)
		ComputeIsLeaf(&i->second, visited);
}

/*
void Grammar::CreateSkipNodes()
{
	Defs::iterator i, iEnd = defs.end();
	for (i = defs.begin(); i != iEnd; ++i)
	{
		int index = 1;
		CreateSkipNodes(&i->second, i->first, index);
	}
}

void Grammar::CreateSkipNodes(Expression* _pExpression, const std::string& _name, int& _index)
{
	if (!_pExpression->isLeaf)
	{
		ExpressionType type = _pExpression->GetType();
		if (IsGroup(type))
		{
			Expression::Group& group = _pExpression->GetGroup();
			if (group.first.isLeaf)
			{
				Expression n;
				AddDef(defs, str(boost::format("%1%_%2%") % _name % _index++), n);
			}
			CreateSkipNodes(&group.first, _name, _index);
			CreateSkipNodes(&group.second, _name, _index);
		}
		else if (IsContainer(type))
		{
			Expression& child = _pExpression->GetChild();
			CreateSkipNodes(&child, _name, _index);
		}
	}
	else
	{
		ExpressionType type = _pExpression->GetType();
		if (IsGroup(type))
		{
			Expression::Group& group = expr.GetGroup();
			CreateSkipNodes(&group.first, _name, _index);
			CreateSkipNodes(&group.second, _name, _index);
		}
		else if (IsContainer(type))
		{
			Expression& child = _pExpression->GetChild();
			CreateSkipNodes(&child, _name, _index);
		}
	}
	
	while (pExpression->GetType() == ExpressionType_Choice)
	{
		Expression::Group& group = pExpression->GetGroup();
		if (group.first.isLeaf)
		
	}
	
		//Group
		case ExpressionType_Choice:
		case ExpressionType_Sequence:
		{
			Expression::Group& group = expr.GetGroup();
			if (group.first.isLeaf)
			{
			}
			ComputeIsLeaf(&group.first, _visited);
			ComputeIsLeaf(&group.second, _visited);
			_pExpression->isLeaf = group.first.isLeaf && group.second.isLeaf;
			break;
		}
			
		//Container
		case ExpressionType_ZeroOrMore:
		{
			Expression& child = _pExpression->GetChild();
			ComputeIsLeaf(&child, _visited);
			_pExpression->isLeaf = child.isLeaf;
			break;
		}
			
		case ExpressionType_Not:

		//SingleChar
		case ExpressionType_Range:
		case ExpressionType_Char:
		case ExpressionType_Dot:
		{
			_pExpression->isLeaf = true;
			break;
		}
			
		//NonTerminal
		case ExpressionType_NonTerminal:
		{
			const std::string& nonTerminal = _pExpression->GetNonTerminal();
			Defs::iterator iDef = defs.find(nonTerminal);
			if (iDef == defs.end())
				throw std::runtime_error(str(boost::format("Non-terminal not found: %1%") % nonTerminal));
			
			DefValue* pDefValue = &iDef->second;
			if (pDefValue->isNode)
			{
				_pExpression->isLeaf = false;
			}
			else
			{
				if (_visited.insert(pDefValue).second)
					ComputeIsLeaf(pDefValue, _visited);
			
				_pExpression->isLeaf = pDefValue->isLeaf;
			}
			
			break;
		}
			
		default:
		{
			assert(false);
			break;
		}
	}	
}
*/

void Grammar::ComputeIsLeaf(Expression* _pExpression, std::set<DefValue*>& _visited)
{
	switch (_pExpression->GetType())
	{
		case ExpressionType_Empty:
		{
			_pExpression->isLeaf = true;
			break;
		}
			
		//Group
		case ExpressionType_Choice:
		case ExpressionType_Sequence:
		{
			Expression::Group& group = _pExpression->GetGroup();
			ComputeIsLeaf(&group.first, _visited);
			ComputeIsLeaf(&group.second, _visited);
			_pExpression->isLeaf = group.first.isLeaf && group.second.isLeaf;
			break;
		}
			
		//Container
		case ExpressionType_ZeroOrMore:
		{
			Expression& child = _pExpression->GetChild();
			ComputeIsLeaf(&child, _visited);
			_pExpression->isLeaf = child.isLeaf;
			break;
		}
			
		case ExpressionType_Not:

		//SingleChar
		case ExpressionType_Range:
		case ExpressionType_Char:
		case ExpressionType_Dot:
		{
			_pExpression->isLeaf = true;
			break;
		}
			
		//NonTerminal
		case ExpressionType_NonTerminal:
		{
			const std::string& nonTerminal = _pExpression->GetNonTerminal();
			Defs::iterator iDef = defs.find(nonTerminal);
			if (iDef == defs.end())
				throw std::runtime_error(str(boost::format("Non-terminal not found: %1%") % nonTerminal));
			
			DefValue* pDefValue = &iDef->second;
			if (pDefValue->isNode)
			{
				_pExpression->isLeaf = false;
			}
			else
			{
				if (_visited.insert(pDefValue).second)
					ComputeIsLeaf(pDefValue, _visited);
			
				_pExpression->isLeaf = pDefValue->isLeaf;
			}
			
			break;
		}
			
		default:
		{
			assert(false);
			break;
		}
	}	
}
