/*
 * ConvertExpression.cpp
 *
 *  Created on: 07-04-2009
 *      Author: alexk7
 */

#include "../Common.h"
#include "ConvertExpression.h"
#include "ConvertIdentifier.h"
#include "../AST/Expression.h"
#include "../PT/Symbol.h"
#include "../PT/Visitor.h"

struct CharVisitor : Visitor
{
	char c;
	void Visit(Symbol* _symbol, SymbolType _type)
	{
		if (_type == SymbolType_Char_1)
		{
			++_symbol;
			if (_symbol->value == 'n')
			{
				c = '\n';
			}
			else if (_symbol->value == 'r')
			{
				c = '\r';
			}
			else if (_symbol->value == 't')
			{
				c = '\t';
			}
			else if (_symbol->value >= '1' && _symbol->value <= '9')
			{
				c = 0;
				Symbol* symbolEnd = _symbol->result[SymbolType_Char_1_1_2];
				for (;;)
				{
					c += _symbol->value - '0';
					if (++_symbol == symbolEnd)
						break;
					c *= 10;
				}
			}
			else
			{
				c = _symbol->value;
			}
		}
		else if (_type == SymbolType_Char_2)
		{
			c = _symbol->value;
		}
	}
};

struct ExpressionBuilder : Visitor
{
	Expression expression;
};

struct LiteralVisitor : ExpressionBuilder
{
	void Visit(Symbol* _symbol, SymbolType _type)
	{
		if (_type == SymbolType_Char)
		{
			CharVisitor visitor;
			VisitChildren(_symbol, SymbolType_Char, visitor);

			Expression charExpression;
			charExpression.SetChar(visitor.c);
			expression.AddGroupItem(ExpressionType_Sequence, charExpression);
		}
	}
};

struct ClassVisitor : ExpressionBuilder
{
	void Visit(Symbol* _symbol, SymbolType _type)
	{
		if (_type == SymbolType_Range)
		{
			Expression range;
			CharVisitor firstVisitor;
			VisitChildren(_symbol, SymbolType_Char, firstVisitor);
			if (_symbol->value == '-')
			{
				++_symbol;
				CharVisitor lastVisitor;
				VisitChildren(_symbol, SymbolType_Char, lastVisitor);
				range.SetRange(firstVisitor.c, lastVisitor.c);
			}
			else
			{
				range.SetChar(firstVisitor.c);
			}

			expression.AddGroupItem(ExpressionType_Choice, range);
		}
	}
};

struct PrimaryVisitor : ExpressionBuilder
{
	void Visit(Symbol* _symbol, SymbolType _type)
	{
		switch(_type)
		{
			case SymbolType_Identifier:
			{
				std::string identifier;
				ConvertIdentifier(identifier, _symbol);
				expression.SetNonTerminal(identifier);
				break;
			}
			case SymbolType_Expression:
			{
				ConvertExpression(expression, _symbol);
				break;
			}
			case SymbolType_Literal:
			{
				LiteralVisitor visitor;
				VisitChildren(_symbol, SymbolType_Literal, visitor);
				expression.Swap(visitor.expression);
				break;
			}
			case SymbolType_Class:
			{
				ClassVisitor visitor;
				VisitChildren(_symbol, SymbolType_Class, visitor);
				expression.Swap(visitor.expression);
				break;
			}
			case SymbolType_DOT:
			{
				expression.SetDot();
				break;
			}
            default: /* don't care */ break;
		}
	}
};

struct SuffixVisitor : ExpressionBuilder
{
	void Visit(Symbol* _symbol, SymbolType _type)
	{
		switch (_type)
		{
			case SymbolType_Primary:
			{
				PrimaryVisitor visitor;
				VisitChildren(_symbol, SymbolType_Primary, visitor);
				expression.Swap(visitor.expression);
				break;
			}
			case SymbolType_QUESTION:
			{
				expression.SetContainer(ExpressionType_Optional, expression);
				break;
			}
			case SymbolType_STAR:
			{
				expression.SetContainer(ExpressionType_ZeroOrMore, expression);
				break;
			}
			case SymbolType_PLUS:
			{
				Expression zeroOrMore = expression;
				zeroOrMore.SetContainer(ExpressionType_ZeroOrMore, zeroOrMore);
				expression.AddGroupItem(ExpressionType_Sequence, zeroOrMore);
				break;
			}
            default: /* don't care */ break;
		}
	}
};

struct PrefixVisitor : ExpressionBuilder
{
	ExpressionType containerType;
	PrefixVisitor() : containerType(ExpressionType_Empty) {}
	void Visit(Symbol* _symbol, SymbolType _type)
	{
		switch (_type)
		{
			case SymbolType_AND: containerType = ExpressionType_And; break;
			case SymbolType_NOT: containerType = ExpressionType_Not; break;
			case SymbolType_Suffix:
			{
				SuffixVisitor visitor;
				VisitChildren(_symbol, SymbolType_Suffix, visitor);
				expression.SetContainer(containerType, visitor.expression);
				break;
			}
            default: /* don't care */ break;
		}
	}
};

struct SequenceVisitor : ExpressionBuilder
{
	void Visit(Symbol* _symbol, SymbolType _type)
	{
		if (_type == SymbolType_Prefix)
		{
			PrefixVisitor visitor;
			VisitChildren(_symbol, SymbolType_Prefix, visitor);
			expression.AddGroupItem(ExpressionType_Sequence, visitor.expression);
		}
	}
};

struct ExpressionVisitor : ExpressionBuilder
{
	void Visit(Symbol* _symbol, SymbolType _type)
	{
		if (_type == SymbolType_Sequence)
		{
			SequenceVisitor visitor;
			VisitChildren(_symbol, SymbolType_Sequence, visitor);
			expression.AddGroupItem(ExpressionType_Choice, visitor.expression);
		}
	}
};

void ConvertExpression(Expression& _expression, Symbol* _symbol)
{
	ExpressionVisitor visitor;
	VisitChildren(_symbol, SymbolType_Expression, visitor);
	_expression.Swap(visitor.expression);
}
