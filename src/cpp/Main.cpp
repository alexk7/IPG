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

static bool ReadFile(std::vector<Node>& _symbols, const char* _filename)
{
	_symbols.clear();
	
	std::vector<char> text;
	if (ReadFile(text, _filename))
	{
		std::size_t size = text.size();
		_symbols.resize(size+1);
		
		for (size_t i = 0; i < size; ++i)
		{
			_symbols[i].value = text[i];
		}
		
		_symbols[size].value = 0;
		
		return true;
	}
	
	return false;
}

static char GetChar(PTItr _iChar)
{
	Node* p = _iChar.Begin();
	char c = p->value;
	if (c == '\\')
	{
		c = (++p)->value;
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
				char digit = (++p)->value;
				if (digit < '0' || digit > '9')
					break;
				c = (10 * c) + (digit - '0');
			}
		}
	}
	return c;
}

static void ConvertExpression(Expression& _expr, PTItr _iExpr)
{
	Expression sequence, primary, charExpr;
	
	for (PTItr iSeq = _iExpr.GetChild(SymbolType_Sequence); iSeq; ++iSeq)
	{
		for (PTItr iPrefix = iSeq.GetChild(SymbolType_Prefix); iPrefix; ++iPrefix)
		{
			char cPrefix = (iPrefix.Begin())->value;
			PTItr iSuffix = iPrefix.GetChild(SymbolType_Suffix);
			PTItr iPrimary = iSuffix.GetChild(SymbolType_Primary);
			
			if (PTItr iId = iPrimary.GetChild(SymbolType_Identifier))
			{
				primary.SetNonTerminal(boost::lexical_cast<std::string>(iId));
			}
			else if (PTItr iExpr = iPrimary.GetChild(SymbolType_Expression))
			{
				ConvertExpression(primary, iExpr);
			}
			else if (PTItr iLiteral = iPrimary.GetChild(SymbolType_Literal))
			{
				for (PTItr iChar = iLiteral.GetChild(SymbolType_Char); iChar; ++iChar)
				{
					charExpr.SetChar(GetChar(iChar));
					primary.AddGroupItem(ExpressionType_Sequence, charExpr);
				}
			}
			else if (PTItr iClass = iPrimary.GetChild(SymbolType_Class))
			{
				for (PTItr iRange = iClass.GetChild(SymbolType_Range); iRange; ++iRange)
				{
					PTItr iChar = iRange.GetChild(SymbolType_Char);
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
			
			char cSuffix = (iPrimary.End())->value;
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

static void ConvertGrammar(Grammar& _grammar, PTItr _iGrammar)
{
	for (PTItr iDef = _iGrammar.GetChild(SymbolType_Definition); iDef; ++iDef)
	{
		PTItr iId = iDef.GetChild(SymbolType_Identifier);
		PTItr iArrow = iId.GetNext(SymbolType_LEFTARROW);
		PTItr iExpr = iArrow.GetNext(SymbolType_Expression);
		
		Expression expr;
		ConvertExpression(expr, iExpr);
		
		Def& newDef = AddDef(_grammar.defs, boost::lexical_cast<std::string>(iId), expr);
		char arrowType = (iArrow.Begin() + 1)->value;
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
		std::vector<Node> nodes;
		if (ReadFile(nodes, argv[1]))
		{
			RegisterCPlusPlusTemplates();

			std::string folder = argv[2];
			std::string name = argv[3];
			
			Node* pGrammar = &nodes.front();
			//Print(std::cout, SymbolType_Grammar, pGrammar);
			
			Grammar grammar;
			ConvertGrammar(grammar, PTItr(SymbolType_Grammar, pGrammar));
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
