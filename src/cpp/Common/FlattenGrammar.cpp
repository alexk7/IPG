#include "Common.h"
#include "FlattenGrammar.h"
#include "Grammar.h"

static bool ReferenceAnyNode(const Defs& _defs, const Expression& _expr, std::set<std::string>& _visited)
{
	switch (_expr.GetType())
	{
		case ExpressionType_Empty:
			return false;
			
		//Group
		case ExpressionType_Choice:
		case ExpressionType_Sequence:
		{
			const Expression::Group& group = _expr.GetGroup();
			if (ReferenceAnyNode(_defs, group.first, _visited))
				return true;
			if (ReferenceAnyNode(_defs, group.second, _visited))
				return true;
				
			return false;
		}
			
		//Container
		case ExpressionType_Not:
			return false;

		case ExpressionType_ZeroOrMore:
			return ReferenceAnyNode(_defs, _expr.GetChild(), _visited);
			
		//SingleChar
		case ExpressionType_Range:
		case ExpressionType_Char:
		case ExpressionType_Dot:
			return false;
			
		//NonTerminal
		case ExpressionType_NonTerminal:
		{
			const std::string& nonTerminal = _expr.GetNonTerminal();
			if (_visited.find(nonTerminal) != _visited.end())
				return false;
			const Def& def = *_defs.find(nonTerminal);
			if (def.second.isNode)
				return true;
			std::set<std::string>::iterator iVisited = _visited.insert(nonTerminal).first;
			bool result = ReferenceAnyNode(_defs, def.second, _visited);
			_visited.erase(iVisited);
			return result;
		}
			
		default:
		{
			assert(false);
			return false;
		}
	}
}

static bool ReferenceAnyNode(const Defs& _defs, const Expression& _expr)
{
	std::set<std::string> visited;
	return ReferenceAnyNode(_defs, _expr, visited);
}

static int FlattenExpression(Defs& _defs, const std::string& _name,
                             Expression& _expression, int _index = 1);
static void CreateNonTerminal(Defs& _defs, const std::string& _name,
                              Expression& _expression, int _index)
{
	std::ostringstream oss;
	oss << _name << "_" << _index;
	std::string newName = oss.str();
	
	Def& newDef = AddDef(_defs, newName, _expression);
	_expression.SetNonTerminal(newName);
	newDef.second.isMemoized = true;
	newDef.second.isNodeRef = true;
	
	FlattenExpression(_defs, newName, newDef.second);
}

static int FlattenChild(Defs& _defs, const std::string& _name, ExpressionType _parentType, Expression& _child, int _index)
{
	if (_parentType == ExpressionType_Choice)
	{
		ExpressionType childType = _child.GetType();
		if (childType == ExpressionType_NonTerminal)
		{
			const std::string& nonTerminal = _child.GetNonTerminal();
			_defs.find(nonTerminal)->second.isMemoized = true;
			return _index;
		}
		else if (ReferenceAnyNode(_defs, _child))
		{
			CreateNonTerminal(_defs, _name, _child, _index++);
			return _index;
		}
	}
	
	return FlattenExpression(_defs, _name, _child, _index);
}

static int FlattenExpression(Defs& _defs, const std::string& _name, Expression& _expression, int _index)
{
	ExpressionType type = _expression.GetType();
	if (IsGroup(type))
	{
		Expression::Group& group = _expression.GetGroup();
		_index = FlattenChild(_defs, _name, type, group.first, _index);
		_index = FlattenChild(_defs, _name, type, group.second, _index);
	}
	else if (IsContainer(type))
	{
		Expression& child = _expression.GetChild();
		ExpressionType childType = child.GetType();
		if (childType == ExpressionType_NonTerminal)
			_defs.find(child.GetNonTerminal())->second.isMemoized = true;
		else if (ReferenceAnyNode(_defs, child))
			CreateNonTerminal(_defs, _name, child, _index++);
		else
			_index = FlattenExpression(_defs, _name, child, _index);
	}
	return _index;
}

void FlattenGrammar(Grammar& _grammar)
{
	//This will iterate through newly inserted defs because they happen to
	//be after the one being treated but there is no harm because:
	//1) FlattenExpression is a no-op for already flattened expressions.
	//2) Insertion in a std::map does not invalidate any iterators.
	Defs::iterator i, iEnd = _grammar.defs.end();
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
	{
		bool isNodeRef = ReferenceAnyNode(_grammar.defs, i->second);
		i->second.isNodeRef = isNodeRef;
		if (isNodeRef)
			FlattenExpression(_grammar.defs, i->first, i->second);
	}
}
