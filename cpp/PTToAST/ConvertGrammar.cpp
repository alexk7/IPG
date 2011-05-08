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
#include "../PT/Symbol.h"
#include "../AST/Grammar.h"

struct DefinitionVisitor : Visitor {
	std::string name;
	Expression value;
    bool isNode;
    bool isMemoized;
    DefinitionVisitor() : isNode(false), isMemoized(false) {}
    void Visit(Symbol* _symbol, SymbolType _type) {
        switch (_type) {
            case SymbolType_Identifier: ConvertIdentifier(name, _symbol); break;
            case SymbolType_LEFTARROW:
            {
                char arrowType = _symbol[1].value;
                if (arrowType == '=')
                    isNode = true;
                if (arrowType == '=' || arrowType == '<')
                    isMemoized = true;
                break;
            }
            case SymbolType_Expression: ConvertExpression(value, _symbol); break;
            default: /* don't care */ break;
        }
    }
};

struct GrammarVisitor : Visitor {
	Grammar grammar;
    void Visit(Symbol* _symbol, SymbolType _type) {
        if (_type == SymbolType_Definition) {
        	DefinitionVisitor visitor;
        	VisitChildren(_symbol, SymbolType_Definition, visitor);
            Def& newDef = AddDef(grammar.defs, visitor.name, visitor.value);
            newDef.second.isNode = visitor.isNode;
            newDef.second.isMemoized = visitor.isMemoized;
        }
    }
};

void ConvertGrammar(Grammar& _grammar, Symbol* _symbol)
{
    GrammarVisitor visitor;
	VisitChildren(_symbol, SymbolType_Grammar, visitor);
	std::swap(_grammar, visitor.grammar);
}
