/*
 * ipg.cpp
 *
 *  Created on: 29-03-2009
 *      Author: alexk7
 */

#include "Common.h"
#include "PT/Symbol.h"
#include "PT/Visitor.h"
#include "AST/Grammar.h"
#include "PTToAST/ConvertGrammar.h"
#include "Parser.h"
#include "GenerateParser.h"
#include "FlattenGrammar.h"

const char* symbolName[] =
{
    "Grammar",
    "Definition",
    "Expression",
    "Sequence",
    "Prefix",
    "Suffix",
    "Primary",
    "Identifier",
    "IdentStart",
    "IdentCont",
    "Literal",
    "Class",
    "Range",
    "Char",
    "LEFTARROW",
    "SLASH",
    "AND",
    "NOT",
    "QUESTION",
    "STAR",
    "PLUS",
    "OPEN",
    "CLOSE",
    "DOT",
    "Spacing",
    "Comment",
    "Space",
    "EndOfLine",
    "EndOfFile",
    "Expression_1",
    "Prefix_1",
    "Suffix_1",
    "Primary_1",
    "Primary_2",
    "Literal_1",
    "Literal_1_1",
    "Literal_2",
    "Literal_2_1",
    "Class_1",
    "Range_1",
    "Char_1",
    "Char_1_1",
    "Char_1_1_1",
    "Char_1_1_2",
    "Char_1_1_2_1",
    "Char_1_2",
    "Char_1_2_1",
    "Char_2",
    "Spacing_1",
    "Comment_1"
};

struct TreePrinter : Visitor
{
    int indent;
    TreePrinter(int _indent = 0) : indent(_indent) {}
    void Visit(Symbol* _pSymbol, SymbolType _type)
    {
		if (_type >= SymbolType_Expression_1)
			return;
		if (_type == SymbolType_Space)
			return;
		if (_type == SymbolType_IdentStart)
			return;
		if (_type == SymbolType_IdentCont)
			return;

		Symbol* pSymbolEnd = _pSymbol->result[_type];
		assert(pSymbolEnd);

		if (pSymbolEnd >= _pSymbol + 2) //don't show symbol of less than 2 characters
		{
			for (int i = 0; i < indent; ++i)
				std::cout << ' ';

			std::cout << symbolName[_type] << ": ";

			int len = 0;
			for (Symbol* pSymbol = _pSymbol; pSymbol != pSymbolEnd; ++pSymbol)
			{
				if (++len > 50)
				{
					std::cout << "...";
					break;
				}

				if (pSymbol->value == '\n' || pSymbol->value == '\r')
					std::cout << "\\n";
				else
					std::cout << pSymbol->value;
			}
			std::cout << std::endl;
		}

		TreePrinter childrenPrinter(indent + 1);
		VisitChildren(_pSymbol, _type, childrenPrinter);
		assert(_pSymbol == pSymbolEnd);
    }
};

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

bool ReadFile(std::vector<Symbol>& _symbols, const char* _filename)
{
	std::vector<char> text;
    if (ReadFile(text, _filename))
    {
		std::size_t size = text.size();
		_symbols.resize(size+1);

		for (size_t i = 0; i < size; ++i)
		{
			_symbols[i].value = text[i];
			_symbols[i].result.clear();
		}

		_symbols[size].value = 0;
		_symbols[size].result.clear();

		return true;
    }

    return false;
}

int main(int argc, char* argv[])
{
	if (argc < 4)
		return 1;

	std::vector<Symbol> symbols;
    if (ReadFile(symbols, argv[1]))
    {
        bool bParsed = (Parse(SymbolType_Grammar, &symbols[0]) != NULL);

        std::cout << "Parsing " << (bParsed ? "succeeded" : "failed") << ".\n";
        if (bParsed)
        {
            Grammar grammar;
            ConvertGrammar(grammar, &symbols[0]);
            
            FlattenGrammar(grammar);
 			GenerateParserSource(argv[2], argv[3], grammar);
 			GenerateParserHeader(argv[2], argv[3], grammar);
        }
    }
    return 0;
}

