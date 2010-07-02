/*
 * ipg.cpp
 *
 *  Created on: 29-03-2009
 *      Author: alexk7
 */

#include "Common.h"
#include "PT/Symbol.h"
#include "AST/Grammar.h"
#include "PTToAST/ConvertGrammar.h"
#include "Parser.h"
#include "GenerateParser.h"

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
            std::cout << grammar;
            
            GenerateParserSource(argv[2], argv[3], grammar);
            GenerateParserHeader(argv[2], argv[3], grammar);
        }
    }
    return 0;
}

