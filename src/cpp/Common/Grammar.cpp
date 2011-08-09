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
		else if (i->second.isMemoized)
			_os << "<";
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

void Grammar::CreateSkipNodes()
{
	Defs::iterator i, iEnd = defs.end();
	for (i = defs.begin(); i != iEnd; ++i)
	{
		int index = 1;
		CreateSkipNodes(i->second, i->first, index);
	}
}

void Grammar::CreateSkipNode(Expression& _refExpr, Expression& _skipExpr, const std::string& _name, int& _index)
{
	if (_skipExpr.IsSimple(*this))
	{
		_refExpr.Swap(_skipExpr);
	}
	else
	{
		std::string skipName = str(boost::format("%1%_%2%") % _name % _index++);
		
		DefValue& defValue = AddDef(defs, skipName, _skipExpr).second;
		defValue.isMemoized = true;
		defValue.isLeaf = true;
		
		_refExpr.SetNonTerminal(skipName);
	}
}

void Grammar::CreateSkipNodes(Expression& _expr, const std::string& _name, int& _index)
{
	if (_expr.isLeaf)
		return;
		
	ExpressionType type = _expr.GetType();
	if (IsGroup(type))
	{
		Expression::Group* pGroup = &_expr.GetGroup();
		if (pGroup->first.isLeaf)
		{
			Expression skipExpr;
			for (;;)
			{
				skipExpr.AddGroupItem(type, pGroup->first);
				
				Expression second;
				second.Swap(pGroup->second);
				_expr.Swap(second);
				
				if (_expr.GetType() != type)
					break;
					
				pGroup = &_expr.GetGroup();
				if (!pGroup->first.isLeaf)
					break;
			}
			
			Expression refExpr;
			CreateSkipNode(refExpr, skipExpr, _name, _index);

			if (_expr.isLeaf)
				CreateSkipNode(_expr, _expr, _name, _index);
			else
				CreateSkipNodes(_expr, _name, _index);
					
			Expression newGroup;
			newGroup.AddGroupItem(type, refExpr);
			newGroup.AddGroupItem(type, _expr);
			
			_expr.Swap(newGroup);
		}
		else
		{
			CreateSkipNodes(pGroup->first, _name, _index);
			
			if (pGroup->second.isLeaf)
				CreateSkipNode(pGroup->second, pGroup->second, _name, _index);
			else
				CreateSkipNodes(pGroup->second, _name, _index);
		}
	}
	else if (IsContainer(type))
	{
		CreateSkipNodes(_expr.GetChild(), _name, _index);
	}
}

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
