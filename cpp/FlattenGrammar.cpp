/*
 *  FlattenGrammar.cpp
 *  ipg
 *
 *  Created by Alexandre Cossette on 09-12-12.
 *  Copyright 2009 alexk7. All rights reserved.
 *
 */

#include "Common.h"
#include "FlattenGrammar.h"
#include "AST/Grammar.h"

static void FlattenDefinition(Grammar& _grammar, const std::string& _name, Expression& _expression);
static void FlattenExpression(Grammar& _grammar, const std::string& _name, int& _index, Expression& _expression)
{
	using std::swap;

	std::ostringstream oss;
	oss << _name << "_" << _index++;
	std::string newName = oss.str();

	Expression& newExpression = _grammar.subDefs[newName];
	swap(newExpression, _expression);
	_expression.SetNonTerminal(newName);

	FlattenDefinition(_grammar, newName, newExpression);
}

static void FlattenDefinition(Grammar& _grammar, const std::string& _name, Expression& _expression)
{
	int index = 1;

    if (IsGroup(_expression.GetType()))
    {
        Expressions& children = _expression.GetChildren();
        size_t childCount = children.size();
        for (size_t i = 0; i < childCount; ++i)
        {
            Expression& child = children[i];
            ExpressionType childType = child.GetType();
            if (IsGroup(childType))
                FlattenExpression(_grammar, _name, index, child);
            else if (IsContainer(childType))
                FlattenExpression(_grammar, _name, index, child.GetChild());
        }
    }
	else if (IsContainer(_expression.GetType()))
	{
		FlattenExpression(_grammar, _name, index, _expression.GetChild());
	}
}

void FlattenGrammar(Grammar& _grammar)
{
	using std::swap;
	Grammar flatGrammar;

	Grammar::Defs::iterator i, iEnd = _grammar.defs.end();
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
	{
		FlattenDefinition(flatGrammar, i->first, i->second);
		swap(flatGrammar.defs[i->first], i->second);
	}

	swap(_grammar, flatGrammar);
}
