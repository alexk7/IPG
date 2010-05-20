/*
 *  ipg2.cpp
 *
 *  Created by Alexandre Cossette on 10-02-27.
 *  Copyright 2010 alexk7. All rights reserved.
 *
 */

#include "../cpp/Common.h"
#include "PEGParser.h"

bool ReadFile(std::vector<char>& _text, const char* _filename)
{
	std::ifstream file(_filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		// If the file offset at the end of the file is bigger than the maximum
		// size_t value in bytes (usually 4GB on 32-bit machines), it means the file
		// won't fit in memory even if the file system supports such files!
		std::ifstream::pos_type lastpos = file.tellg();
		file.seekg(0, std::ios::beg);
		std::ifstream::pos_type firstpos = file.tellg();
		std::size_t textsize = static_cast<std::size_t>(lastpos - firstpos);
		
		if (firstpos + std::ifstream::off_type(textsize) != lastpos)
			return false;
		
		_text.resize(textsize);
		file.read(&_text[0], textsize);
		file.close();
		
		return true;
	}
	
	return false;
}

bool ReadFile(std::vector<PTNode>& _symbols, const char* _filename)
{
	std::vector<char> text;
	if (ReadFile(text, _filename))
	{
		std::size_t size = text.size();
		_symbols.resize(size+1);
		
		for (size_t i = 0; i < size; ++i)
		{
			_symbols[i].value = text[i];
			_symbols[i].end.Clear();
		}
		
		_symbols[size].value = 0;
		_symbols[size].end.Clear();
		
		return true;
	}
	
	return false;
}

//PTItr_Definition_Identifier::operator-> return a pointer to its contained PTItr_Identifier
//(it is a pointer just because of C++ syntax rules)
//PTItr_Expression_Sequence must contain an additional index parameter to know which occurence of Sequence inside Expression
//SymbolItr is like Symbol* except that * returns pSymbol->value (for now)

/*
char GetChar(PTItr_Char _iChar)
{
	SymbolItr iSymbol = _iChar.Begin();
	char c = *iSymbol;
	if (c == '\\')
	{
		c = *++iSymbol;
		if (c == 'n')
		{
			c = '\n';
		}
		else if (c == 'r')
		{
			c = '\r';
		}
		else if (c == 't')
		{
			c = '\t';
		}
		else if (c >= '1' && c <= '9')
		{
			c -= '0';
			for (;;)
			{
				char digit = *++iSymbol;
				if (digit < '0' || digit > '9')
					break;
				c = (10 * c) + (digit - '0');
			}
		}
	}
	return c;
}

void ConvertExpression(Expression& _expr, PTItr_Expression _iExpr)
{
	Expression sequence, primary, charExpr;

	for (PTItr_Expression_Sequence iSeq(iExpr); iSeq; ++iSeq)
	{
		for (PTItr_Sequence_Prefix iPrefix(iSeq); iPrefix; ++iPrefix)
		{
			char cPrefix = *iPrefix.Begin();
			PTItr_Prefix_Suffix iSuffix(iPrefix);
			PTItr_Suffix_Primary iPrimary(iSuffix);
			
			if (PTItr_Primary_Identifier iId(iPrimary))
			{
				primary.SetNonTerminal(iId.ToString());
			}
			else if (PTItr_Primary_Expression iExpr(iPrimary))
			{
				ConvertExpression(primary, iExpr);
			}
			else if (PTItr_Primary_Literal iLiteral(iPrimary))
			{
				for (PTItr_Literal_Char iChar(iLiteral); iChar; ++iChar)
				{
					_expr.SetChar(GetChar(iChar));
					primary.AddGroupItem(ExpressionType_Sequence, charExpr);
				}
			}
			else if (PTItr_Primary_Class iClass(iPrimary))
			{
				for (PTItr_Class_Range iRange(iClass); iRange; ++iRange)
				{
					PTItr_Range_Char iChar(iRange);
					char firstChar = GetChar(iChar);
					if (++iChar)
					{
						char lastChar = GetChar(iChar);
						charExpr.SetRange(firstChar, lastChar);
					}
					else
					{
						charExpr.SetChar(firstChar);
					}
					primary.AddGroupItem(ExpressionType_Choice, charExpr);
				}
			}
			else
			{
				primary.SetDot();
			}
			
			char cSuffix = *iPrimary.End();
			if (cSuffix == '?')
			{
				primary.SetContainer(ExpressionType_Optional, primary);
			}
			else if (cSuffix == '*')
			{
				primary.SetContainer(ExpressionType_ZeroOrMore, primary);
			}
			else if (cSuffix == '+')
			{
				Expression zeroOrMore = primary;
				zeroOrMore.SetContainer(ExpressionType_ZeroOrMore, zeroOrMore);
				primary.AddGroupItem(ExpressionType_Sequence, zeroOrMore);
			}
			
			if (cPrefix == '&')
			{
				primary.SetContainer(ExpressionType_And, primary);
			}
			else if (cPrefix == '!')
			{
				primary.SetContainer(ExpressionType_Not, primary);
			}
			
			sequence.AddGroupItem(ExpressionType_Sequence, primary);
		}
		
		_expr.AddGroupItem(ExpressionType_Choice, sequence);
	}
}

void ConvertGrammar(Grammar& _grammar, PTItr_Grammar _iGrammar)
{
	for (PTItr_Grammar_Definition iDef(_iGrammar); iDef; ++iDef)
	{
		PTItr_Definition_Identifier iDefId(iDef);
		PTItr_Definition_Expression iExpr(iId);
		
		Expression expr;
		ConvertExpression(expr, iExpr);
		
		_grammar[iDefId.ToString()].Swap(expr);
	}
}
//*/

int main(int argc, char* argv[])
{
	if (argc < 4)
		return 1;
	
	std::vector<PTNode> nodes;
	if (ReadFile(nodes, argv[1]))
	{
		bool bParsed = (Parse_Grammar(&nodes[0]) != NULL);
		
		std::cout << "Parsing " << (bParsed ? "succeeded" : "failed") << ".\n";
		/*
		if (bParsed)
		{
			Grammar grammar;
			ConvertGrammar(grammar, &nodes[0]);
			
			FlattenGrammar(grammar);
			GenerateParserSource(argv[2], argv[3], grammar);
			GenerateParserHeader(argv[2], argv[3], grammar);
		}
		*/
	}
	return 0;
}
