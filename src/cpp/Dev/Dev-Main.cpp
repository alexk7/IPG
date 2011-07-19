#include "Common.h"
#include "Grammar.h"
#include "GenerateParser.h"
#include "ReadFile.h"
#include "Parser.h.tpl.h"
#include "Parser.cpp.tpl.h"

#include PEG_PARSER_INCLUDE

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <ctemplate/template.h>

using namespace PEGParser;

static char GetChar(Iterator _iChar)
{
	const char* p = _iChar.Begin();
	char c = *p;
	if (c == '\\')
	{
		c = *++p;
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
				char digit = *++p;
				if (digit < '0' || digit > '9')
					break;
				c = (10 * c) + (digit - '0');
			}
		}
	}
	return c;
}

static void ConvertExpression(Expression& _expr, Iterator _iExpr)
{
	Expression sequence, primary, charExpr;
	
	for (Iterator iSeq = _iExpr.GetChild(SymbolType_Sequence); iSeq; ++iSeq)
	{
		for (Iterator iPrefix = iSeq.GetChild(SymbolType_Prefix); iPrefix; ++iPrefix)
		{
			char cPrefix = *iPrefix.Begin();
			Iterator iSuffix = iPrefix.GetChild(SymbolType_Suffix);
			Iterator iPrimary = iSuffix.GetChild(SymbolType_Primary);
			
			if (Iterator iId = iPrimary.GetChild(SymbolType_Identifier))
			{
				primary.SetNonTerminal(boost::lexical_cast<std::string>(iId));
			}
			else if (Iterator iExpr = iPrimary.GetChild(SymbolType_Expression))
			{
				ConvertExpression(primary, iExpr);
			}
			else if (Iterator iLiteral = iPrimary.GetChild(SymbolType_Literal))
			{
				for (Iterator iChar = iLiteral.GetChild(SymbolType_Char); iChar; ++iChar)
				{
					charExpr.SetChar(GetChar(iChar));
					primary.AddGroupItem(ExpressionType_Sequence, charExpr);
				}
			}
			else if (Iterator iClass = iPrimary.GetChild(SymbolType_Class))
			{
				for (Iterator iRange = iClass.GetChild(SymbolType_Range); iRange; ++iRange)
				{
					Iterator iChar = iRange.GetChild(SymbolType_Char);
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
				Expression empty;
				primary.AddGroupItem(ExpressionType_Choice, empty);
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
				primary.SetContainer(ExpressionType_Not, primary);
				primary.SetContainer(ExpressionType_Not, primary);
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

static void ConvertGrammar(Grammar& _grammar, Iterator _iGrammar)
{
	for (Iterator iDef = _iGrammar.GetChild(SymbolType_Definition); iDef; ++iDef)
	{
		Iterator iId = iDef.GetChild(SymbolType_Identifier);
		Iterator iArrow = iId.GetNext(SymbolType_LEFTARROW);
		Iterator iExpr = iArrow.GetNext(SymbolType_Expression);
		
		Expression expr;
		ConvertExpression(expr, iExpr);
		
		Def& newDef = AddDef(_grammar.defs, boost::lexical_cast<std::string>(iId), expr);
		char arrowType = *(iArrow.Begin() + 1);
		if (arrowType == '=')
			newDef.second.isNode = true;
		if (arrowType == '=' || arrowType == '<')
			newDef.second.isMemoized = true;
	}
	
	_grammar.ComputeIsLeaf();
}

static void RegisterTemplate(const ctemplate::TemplateString&  _key, const ctemplate::TemplateString&  _content)
{
	ctemplate::StringToTemplateCache(_key, _content, ctemplate::STRIP_BLANK_LINES);
}

static void RegisterCPlusPlusTemplates()
{
	RegisterTemplate("Parser.h.tpl", Parser_h_tpl);
	RegisterTemplate("Parser.cpp.tpl", Parser_cpp_tpl);
}

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		std::cerr << "Usage: ipg peg.txt path-prefix name" << std::endl;
		return 1;
	}
	
	try
	{
		std::vector<char> nodes;
		if (ReadFile(nodes, argv[1]))
		{
			nodes.push_back('\0');
			
			RegisterCPlusPlusTemplates();

			std::string folder = argv[2];
			std::string name = argv[3];
			
			Iterator iGrammar(SymbolType_Grammar, &nodes.front());
			iGrammar.Print(std::cout);
			
			Grammar grammar;
			ConvertGrammar(grammar, iGrammar);
			//std::cout << grammar;
			
			GenerateParser(argv[1], folder, name, grammar);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
		return 1;
	}
	
	return 0;
}
