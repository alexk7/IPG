#include "Common.h"
#include "Grammar.h"
#include "GenerateParser.h"
#include "ReadFile.h"
#include "Parser.h.tpl.h"
#include "Parser.cpp.tpl.h"

#ifdef IPG_DEV
#include "Dev-PEGParser.h"
#else
#include "PEGParser.h"
#endif

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <ctemplate/template.h>

using namespace PEGParser;

static char GetChar(Iterator _iChar)
{
	const char* p = _iChar->value;
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
	assert(_iExpr->type == SymbolType_Expression);
	Expression sequence, primary, charExpr;
	
	for (Iterator iSeq = _iExpr.GetChild(); iSeq->type == SymbolType_Sequence; ++iSeq)
	{
		for (Iterator iItem = iSeq.GetChild(); iItem->type == SymbolType_Item; ++iItem)
		{
			char cPrefix = *iItem->value;
			
			Iterator iPrimary = iItem.GetChild();
			assert(iPrimary->type == SymbolType_Primary);
			Iterator i = iPrimary.GetChild();
			if (i->type == SymbolType_Identifier)
			{
				primary.SetNonTerminal(std::string(i->value, i->length));
			}
			else if (i->type == SymbolType_Expression)
			{
				ConvertExpression(primary, i);
			}
			else if (i->type == SymbolType_LITERAL)
			{
				for (Iterator iChar = i.GetChild(); iChar->type == SymbolType_Char; ++iChar)
				{
					charExpr.SetChar(GetChar(iChar));
					primary.AddGroupItem(ExpressionType_Sequence, charExpr);
				}
			}
			else if (i->type == SymbolType_CLASS)
			{
				for (Iterator iRange = i.GetChild(); iRange->type == SymbolType_Range; ++iRange)
				{
					Iterator iChar = iRange.GetChild();
					assert(iChar->type == SymbolType_Char);
					char firstChar = GetChar(iChar);
					if ((++iChar)->type == SymbolType_Char)
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
			
			char cSuffix = iPrimary->value[iPrimary->length];
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
				Expression zeroOrMore(primary);
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
	for (Iterator iDef = _iGrammar.GetChild(); iDef->type == SymbolType_Definition; ++iDef)
	{
		Iterator i = iDef.GetChild();
		assert(i->type == SymbolType_Identifier);
		std::string id(i->value, i->length);
		char arrowType = (++i)->value[1];
		
		Expression expr;
		ConvertExpression(expr, ++i);
		
		Def& newDef = AddDef(_grammar.defs, id, expr);
		if (arrowType == '=')
		{
			newDef.second.isNode = true;
			newDef.second.isMemoized = true;
		}
	}
	
	_grammar.ComputeIsLeaf();
	_grammar.CreateSkipNodes();
}

static void RegisterTemplate(const ctemplate::TemplateString&  _key, const ctemplate::TemplateString&  _content)
{
	ctemplate::StringToTemplateCache(_key, _content, ctemplate::STRIP_BLANK_LINES);
}

static std::string WithLineDirectives(const std::string& _code, const std::string& _filename)
{
	std::istringstream iss(_code);
	std::ostringstream oss;
	std::string line;
	int lineNumber = 0;
	while (getline(iss, line))
		oss << "#line " << ++lineNumber << "\"" << _filename << "\"\n" << line << "\n";
		
	return oss.str();
}

static void RegisterCPlusPlusTemplates(bool _withLineDirectives)
{
	if (_withLineDirectives)
	{
		RegisterTemplate("Parser.h.tpl", WithLineDirectives(Parser_h_tpl, "../../src/tpl/Parser.h.tpl"));
		RegisterTemplate("Parser.cpp.tpl", WithLineDirectives(Parser_cpp_tpl, "../../src/tpl/Parser.cpp.tpl"));
	}
	else
	{	
		RegisterTemplate("Parser.h.tpl", Parser_h_tpl);
		RegisterTemplate("Parser.cpp.tpl", Parser_cpp_tpl);
	}
}

int main(int argc, char* argv[])
{
	try
	{
		std::string pegFile;
		std::string folder;
		std::string name;
	
		namespace po = boost::program_options;
		po::options_description options("Options");
		options.add_options()
		("help,h",                          "produce help message")
		("peg-file",   po::value(&pegFile), "text file that contain the parsing expression grammar")
		("dir",        po::value(&folder),  "destination directory for the generated files")
		("name",       po::value(&name),    "name used for the generated files and namespace")
		("no-lines,l",                      "don't put any #line preprocessor commands in the generated files")
		;

		po::positional_options_description positional;
		positional.add("peg-file", 1);
		positional.add("dir", 1);
		positional.add("name", 1);
		
		po::variables_map variablesMap;
		store(po::command_line_parser(argc, argv).options(options).positional(positional).run(), variablesMap);
		notify(variablesMap);

		if (variablesMap.count("help"))
		{
				std::cout << options << "\n";
				return 0;
		}

		std::vector<char> nodes;
		if (ReadFile(nodes, pegFile.c_str()))
		{
			nodes.push_back('\0');
			
			RegisterCPlusPlusTemplates(variablesMap.count("no-lines") == 0);
			
			boost::shared_ptr<Parser> pParser(new Parser);
			Iterator iGrammar(pParser, SymbolType_Grammar, &nodes.front());
			//iGrammar.Print(std::cout);
			
			Grammar grammar;
			ConvertGrammar(grammar, iGrammar);
			std::cout << grammar;
			
			GenerateParser(pegFile, folder, name, grammar);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
		return 1;
	}
	
	return 0;
}
