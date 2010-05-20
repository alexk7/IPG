/*
 * ConvertGrammar.cpp
 *
 *  Created on: 07-04-2009
 *      Author: alexk7
 */

#include "../Common.h"
#include "ConvertGrammar.h"
#include "ConvertIdentifier.h"
#include "ConvertExpression.h"
#include "../PT/Visitor.h"
#include "../AST/Grammar.h"

struct DefinitionVisitor : Visitor {
	std::string name;
	Expression value;
    void Visit(Symbol* _symbol, SymbolType _type) {
        switch (_type) {
            case SymbolType_Identifier: ConvertIdentifier(name, _symbol); break;
            case SymbolType_Expression: ConvertExpression(value, _symbol); break;
        }
    }
};

struct GrammarVisitor : Visitor {
	Grammar grammar;
    void Visit(Symbol* _symbol, SymbolType _type) {
        if (_type == SymbolType_Definition) {
        	DefinitionVisitor visitor;
        	VisitChildren(_symbol, SymbolType_Definition, visitor);
            std::swap(grammar.defs[visitor.name], visitor.value);
        }
    }
};

void ConvertGrammar(Grammar& _grammar, Symbol* _symbol)
{
    GrammarVisitor visitor;
	VisitChildren(_symbol, SymbolType_Grammar, visitor);
	std::swap(_grammar, visitor.grammar);
}
