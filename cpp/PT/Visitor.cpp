/*
 * Visitor.cpp
 *
 *  Created on: 17-08-2009
 *      Author: alexk7
 */

#include "../Common.h"
#include "Visitor.h"
#include "Symbol.h"

static void VisitSymbol(Symbol*& _symbol, SymbolType _type, Visitor& _visitor)
{
    _visitor.Visit(_symbol, _type);
    assert(_symbol->result[_type] != NULL);
    _symbol = _symbol->result[_type];
}

static void VisitOneOrMore(Symbol*& _symbol, SymbolType _type, Visitor& _visitor)
{
    do
        VisitSymbol(_symbol, _type, _visitor);
    while (_symbol->result[_type]);
}

static void VisitZeroOrMore(Symbol*& _symbol, SymbolType _type, Visitor& _visitor)
{
    while (_symbol->result[_type])
        VisitSymbol(_symbol, _type, _visitor);
}

static void VisitOptional(Symbol*& _symbol, SymbolType _type, Visitor& _visitor)
{
    if (_symbol->result[_type])
        VisitSymbol(_symbol, _type, _visitor);
}

struct ChildrenVisitor
{
private:
    struct HiddenVisitor : Visitor
    {
        HiddenVisitor(Visitor& _visitor) : visitor(_visitor) {}
        Visitor& visitor;
        virtual void Visit(Symbol* _symbol, SymbolType _type)
        {
            visitor.Visit(_symbol, _type);
            VisitChildren(_symbol, _type, visitor);
        }
    } hidden;
    
public:
	ChildrenVisitor(Visitor& _visitor) : hidden(_visitor) {}
	operator Visitor&()
    {
        return hidden;
    }
};

void VisitChildren(Symbol*& _symbol, SymbolType _type, Visitor& _visitor)
{
    switch (_type)
    {
        //Grammar <- Spacing Definition+ EndOfFile
        case SymbolType_Grammar:
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            VisitOneOrMore(_symbol, SymbolType_Definition, _visitor);
            VisitSymbol(_symbol, SymbolType_EndOfFile, _visitor);
            break;

        //Definition <- Identifier LEFTARROW Expression
        case SymbolType_Definition:
            VisitSymbol(_symbol, SymbolType_Identifier, _visitor);
            VisitSymbol(_symbol, SymbolType_LEFTARROW, _visitor);
            VisitSymbol(_symbol, SymbolType_Expression, _visitor);
            break;

        //Expression <- Sequence (SLASH Sequence)*
        case SymbolType_Expression:
            VisitSymbol(_symbol, SymbolType_Sequence, _visitor);
            VisitZeroOrMore(_symbol, SymbolType_Expression_1, ChildrenVisitor(_visitor));
            break;
        case SymbolType_Expression_1:
            VisitSymbol(_symbol, SymbolType_SLASH, _visitor);
            VisitSymbol(_symbol, SymbolType_Sequence, _visitor);
            break;

        //Sequence <- Prefix*
        case SymbolType_Sequence:
            VisitZeroOrMore(_symbol, SymbolType_Prefix, _visitor);
            break;

        //Prefix <- (AND / NOT)? Suffix
        case SymbolType_Prefix:
            VisitOptional(_symbol, SymbolType_Prefix_1, ChildrenVisitor(_visitor));
            VisitSymbol(_symbol, SymbolType_Suffix, _visitor);
            break;
        case SymbolType_Prefix_1:
            if (_symbol->result[SymbolType_AND])
                VisitSymbol(_symbol, SymbolType_AND, _visitor);
            else
                VisitSymbol(_symbol, SymbolType_NOT, _visitor);
            break;

        //Suffix <- Primary (QUESTION / STAR / PLUS)?
        case SymbolType_Suffix:
            VisitSymbol(_symbol, SymbolType_Primary, _visitor);
            VisitOptional(_symbol, SymbolType_Suffix_1, ChildrenVisitor(_visitor));
            break;
        case SymbolType_Suffix_1:
            if (_symbol->result[SymbolType_QUESTION])
                VisitSymbol(_symbol, SymbolType_QUESTION, _visitor);
            else if (_symbol->result[SymbolType_STAR])
                VisitSymbol(_symbol, SymbolType_STAR, _visitor);
            else
                VisitSymbol(_symbol, SymbolType_PLUS, _visitor);
            break;

        //Primary <- Identifier !LEFTARROW
        //         / OPEN Expression CLOSE
        //         / Literal / Class / DOT
        case SymbolType_Primary:
            if (_symbol->result[SymbolType_Primary_1])
                VisitSymbol(_symbol, SymbolType_Primary_1, ChildrenVisitor(_visitor));
            else if (_symbol->result[SymbolType_Primary_2])
                VisitSymbol(_symbol, SymbolType_Primary_2, ChildrenVisitor(_visitor));
            else if (_symbol->result[SymbolType_Literal])
                VisitSymbol(_symbol, SymbolType_Literal, _visitor);
            else if (_symbol->result[SymbolType_Class])
                VisitSymbol(_symbol, SymbolType_Class, _visitor);
            else
                VisitSymbol(_symbol, SymbolType_DOT, _visitor);
            break;
        case SymbolType_Primary_1:
            VisitSymbol(_symbol, SymbolType_Identifier, _visitor);
            break;
        case SymbolType_Primary_2:
            VisitSymbol(_symbol, SymbolType_OPEN, _visitor);
            VisitSymbol(_symbol, SymbolType_Expression, _visitor);
            VisitSymbol(_symbol, SymbolType_CLOSE, _visitor);
            break;

        //Identifier <- IdentStart IdentCont* Spacing
        case SymbolType_Identifier:
            VisitSymbol(_symbol, SymbolType_IdentStart, _visitor);
            VisitZeroOrMore(_symbol, SymbolType_IdentCont, _visitor);
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;

        //IdentStart <- [a-zA-Z_]
        case SymbolType_IdentStart:
            ++_symbol;
            break;

        //IdentCont <- IdentStart / [0-9]
        case SymbolType_IdentCont:
            if (_symbol->result[SymbolType_IdentStart])
                VisitSymbol(_symbol, SymbolType_IdentStart, _visitor);
            else
                ++_symbol;
            break;

        //Literal <- ['] (!['] Char)* ['] Spacing
        //         / ["] (!["] Char)* ["] Spacing
        case SymbolType_Literal:
            if (_symbol->result[SymbolType_Literal_1])
                VisitSymbol(_symbol, SymbolType_Literal_1, ChildrenVisitor(_visitor));
            else
                VisitSymbol(_symbol, SymbolType_Literal_2, ChildrenVisitor(_visitor));
            break;
        case SymbolType_Literal_1:
            ++_symbol;
            VisitZeroOrMore(_symbol, SymbolType_Literal_1_1, ChildrenVisitor(_visitor));
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;
        case SymbolType_Literal_1_1:
            VisitSymbol(_symbol, SymbolType_Char, _visitor);
            break;
        case SymbolType_Literal_2:
            ++_symbol;
            VisitZeroOrMore(_symbol, SymbolType_Literal_2_1, ChildrenVisitor(_visitor));
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;
        case SymbolType_Literal_2_1:
            VisitSymbol(_symbol, SymbolType_Char, _visitor);
            break;

        //Class <- '[' (!']' Range)* ']' Spacing
        case SymbolType_Class:
            ++_symbol;
            VisitZeroOrMore(_symbol, SymbolType_Class_1, ChildrenVisitor(_visitor));
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;
        case SymbolType_Class_1:
            VisitSymbol(_symbol, SymbolType_Range, _visitor);
            break;

        //Range <- Char ('-' Char)?
        case SymbolType_Range:
            VisitSymbol(_symbol, SymbolType_Char, _visitor);
            VisitOptional(_symbol, SymbolType_Range_1, ChildrenVisitor(_visitor));
            break;
        case SymbolType_Range_1:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Char, _visitor);
            break;

        //Char <- '\\' ([nrt'"\[\]\\] / [1-9][0-9]*)
        //      / !'\\' .
        case SymbolType_Char:
            if (_symbol->result[SymbolType_Char_1])
                VisitSymbol(_symbol, SymbolType_Char_1, ChildrenVisitor(_visitor));
            else
                VisitSymbol(_symbol, SymbolType_Char_2, ChildrenVisitor(_visitor));
            break;
        case SymbolType_Char_1:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Char_1_1, ChildrenVisitor(_visitor));
            break;
        case SymbolType_Char_1_1:
            if (_symbol->result[SymbolType_Char_1_1_1])
                VisitSymbol(_symbol, SymbolType_Char_1_1_1, ChildrenVisitor(_visitor));
            else
                VisitSymbol(_symbol, SymbolType_Char_1_1_2, ChildrenVisitor(_visitor));
            break;
        case SymbolType_Char_1_1_1:
            ++_symbol;
            break;
        case SymbolType_Char_1_1_2:
            ++_symbol;
            VisitZeroOrMore(_symbol, SymbolType_Char_1_1_2_1, ChildrenVisitor(_visitor));
            break;
        case SymbolType_Char_1_1_2_1:
            ++_symbol;
            break;
        case SymbolType_Char_2:
            ++_symbol;
            break;

        //LEFTARROW <- '<-' Spacing
        case SymbolType_LEFTARROW:
            _symbol += 2;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;
        case SymbolType_LEFTARROW_1:
            assert(false); //not visited
            return;

        //SLASH <- '/' Spacing
        case SymbolType_SLASH:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;

        //AND <- '&' Spacing
        case SymbolType_AND:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;

        //NOT <- '!' Spacing
        case SymbolType_NOT:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;

        //QUESTION <- '?' Spacing
        case SymbolType_QUESTION:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;

        //STAR <- '*' Spacing
        case SymbolType_STAR:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;

        //PLUS <- '+' Spacing
        case SymbolType_PLUS:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;

        //OPEN <- '(' Spacing
        case SymbolType_OPEN:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;

        //CLOSE <- ')' Spacing
        case SymbolType_CLOSE:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;

        //DOT <- '.' Spacing
        case SymbolType_DOT:
            ++_symbol;
            VisitSymbol(_symbol, SymbolType_Spacing, _visitor);
            break;

        //Spacing <- (Space / Comment)*
        case SymbolType_Spacing:
            VisitZeroOrMore(_symbol, SymbolType_Spacing_1, ChildrenVisitor(_visitor));
            break;
        case SymbolType_Spacing_1:
            if (_symbol->result[SymbolType_Space])
                VisitSymbol(_symbol, SymbolType_Space, _visitor);
            else
                VisitSymbol(_symbol, SymbolType_Comment, _visitor);
            break;

        //Comment <- '#' (!EndOfLine .)* EndOfLine
        case SymbolType_Comment:
            ++_symbol;
            VisitZeroOrMore(_symbol, SymbolType_Comment_1, ChildrenVisitor(_visitor));
            VisitSymbol(_symbol, SymbolType_EndOfLine, _visitor);
            break;
        case SymbolType_Comment_1:
            ++_symbol;
            break;

        //Space <- ' ' / '\t' / EndOfLine
        case SymbolType_Space:
            if (_symbol->value == ' ')
                ++_symbol;
            else if (_symbol->value == '\t')
                ++_symbol;
            else
                VisitSymbol(_symbol, SymbolType_EndOfLine, _visitor);
            break;

        //EndOfLine <- '\r\n' / '\n' / '\r'
        case SymbolType_EndOfLine:
            if (_symbol->value == '\r' && (_symbol+1)->value == '\n')
                _symbol += 2;
            else
                ++_symbol;
            break;

        //EndOfFile <- !.
        case SymbolType_EndOfFile:
            break;
            
        case SymbolType_Count:
            assert(false);
            return;
    }
}
