/*
 * Parser.cpp
 *
 *  Created on: 7-11-2009
 *      Author: alexk7
 */

#include "Common.h"
#include "Parser.h"
#include "PT/Symbol.h"

static Symbol* ParseNot(SymbolType _type, Symbol* _first)
{
	if (Parse(_type, _first))
		return NULL;
	return _first;
}

static Symbol* ParseAnd(SymbolType _type, Symbol* _first)
{
	if (Parse(_type, _first))
		return _first;
	return NULL;
}

static Symbol* ParseZeroOrMore(SymbolType _type, Symbol* _first)
{
	Symbol* result;
	while (result = Parse(_type, _first))
		_first = result;
	return _first;
}

static Symbol* ParseOneOrMore(SymbolType _type, Symbol* _first)
{
	Symbol* result;
	if (!(result = Parse(_type, _first)))
        return NULL;
	return ParseZeroOrMore(_type, result);
}

static Symbol* ParseOptional(SymbolType _type, Symbol* _first)
{
	Symbol* result;
	if (result = Parse(_type, _first))
		return result;
	return _first;
}

static Symbol* ParseRange(char _rangeBegin, char _rangeEnd, Symbol* _symbol)
{
	if (_symbol->value < _rangeBegin)
        return NULL;
	if (_symbol->value > _rangeEnd)
        return NULL;
	return ++_symbol;
}

static Symbol* ParseChar(char _char, Symbol* _symbol)
{
	if (_symbol->value != _char) return NULL;
	return ++_symbol;
}

static Symbol* ParseString(const char* _string, Symbol* _first)
{
	while (*_string)
    {
		if (_first->value != *_string)
            return NULL;
		++_string;
		++_first;
	}
	return _first;
}

static Symbol* ParseAnyChar(Symbol* _symbol)
{
	if (_symbol->value == 0)
        return NULL;
	return ++_symbol;
}

Symbol* Parse(SymbolType _type, Symbol* _first)
{
    Symbol* result = _first->result[_type];
    if (result)
        return result;
    switch (_type)
	{
        //Grammar <- Spacing Definition+ EndOfFile
        case SymbolType_Grammar:
            if (!(result = Parse(SymbolType_Spacing, _first))) break;
            if (!(result = ParseOneOrMore(SymbolType_Definition, result))) break;
            result = Parse(SymbolType_EndOfFile, result);
            break;

        //Definition <- Identifier LEFTARROW Expression
        case SymbolType_Definition:
            if (!(result = Parse(SymbolType_Identifier, _first))) break;
            if (!(result = Parse(SymbolType_LEFTARROW, result))) break;
            result = Parse(SymbolType_Expression, result);
            break;

        //Expression <- Sequence (SLASH Sequence)*
        case SymbolType_Expression:
            if (!(result = Parse(SymbolType_Sequence, _first))) break;
            result = ParseZeroOrMore(SymbolType_Expression_1, result);
            break;
        case SymbolType_Expression_1:
            if (!(result = Parse(SymbolType_SLASH, _first))) break;
            result = Parse(SymbolType_Sequence, result);
            break;

        //Sequence <- Prefix*
        case SymbolType_Sequence:
            result = ParseZeroOrMore(SymbolType_Prefix, _first);
            break;

        //Prefix <- (AND / NOT)? Suffix
        case SymbolType_Prefix:
            result = ParseOptional(SymbolType_Prefix_1, _first);
            result = Parse(SymbolType_Suffix, result);
            break;
        case SymbolType_Prefix_1:
            if (result = Parse(SymbolType_AND, _first)) break;
            result = Parse(SymbolType_NOT, _first);
            break;

        //Suffix <- Primary (QUESTION / STAR / PLUS)?
        case SymbolType_Suffix:
            if (!(result = Parse(SymbolType_Primary, _first))) break;
            result = ParseOptional(SymbolType_Suffix_1, result);
            break;
        case SymbolType_Suffix_1:
            if (result = Parse(SymbolType_QUESTION, _first)) break;
            if (result = Parse(SymbolType_STAR, _first)) break;
            result = Parse(SymbolType_PLUS, _first);
            break;

        //Primary <- Identifier !LEFTARROW
        //         / OPEN Expression CLOSE
        //         / Literal / Class / DOT
        case SymbolType_Primary:
            if (result = Parse(SymbolType_Primary_1, _first)) break;
            if (result = Parse(SymbolType_Primary_2, _first)) break;
            if (result = Parse(SymbolType_Literal, _first)) break;
            if (result = Parse(SymbolType_Class, _first)) break;
            result = Parse(SymbolType_DOT, _first);
            break;
        case SymbolType_Primary_1:
            if (!(result = Parse(SymbolType_Identifier, _first))) break;
            result = ParseNot(SymbolType_LEFTARROW, result);
            break;
        case SymbolType_Primary_2:
            if (!(result = Parse(SymbolType_OPEN, _first))) break;
            if (!(result = Parse(SymbolType_Expression, result))) break;
            result = Parse(SymbolType_CLOSE, result);
            break;

        //Identifier <- IdentStart IdentCont* Spacing
        case SymbolType_Identifier:
            if (!(result = Parse(SymbolType_IdentStart, _first))) break;
            result = ParseZeroOrMore(SymbolType_IdentCont, result);
            result = Parse(SymbolType_Spacing, result);
            break;

        //IdentStart <- [a-zA-Z_]
        case SymbolType_IdentStart:
            if (result = ParseRange('a', 'z', _first)) break;
            if (result = ParseRange('A', 'Z', _first)) break;
            result = ParseChar('_', _first);
            break;

        //IdentCont <- IdentStart / [0-9]
        case SymbolType_IdentCont:
            if (result = Parse(SymbolType_IdentStart, _first)) break;
            result = ParseRange('0', '9', _first);
            break;

        //Literal <- ['] (!['] Char)* ['] Spacing
        //         / ["] (!["] Char)* ["] Spacing
        case SymbolType_Literal:
            if (result = Parse(SymbolType_Literal_1, _first)) break;
            result = Parse(SymbolType_Literal_2, _first);
            break;
        case SymbolType_Literal_1:
            if (!(result = ParseChar('\'', _first))) break;
            result = ParseZeroOrMore(SymbolType_Literal_1_1, result);
            if (!(result = ParseChar('\'', result))) break;
            result = Parse(SymbolType_Spacing, result);
            break;
        case SymbolType_Literal_1_1:
            if (ParseChar('\'', _first)) break;
            result = Parse(SymbolType_Char, _first);
            break;
        case SymbolType_Literal_2:
            if (!(result = ParseChar('\"', _first))) break;
            result = ParseZeroOrMore(SymbolType_Literal_2_1, result);
            if (!(result = ParseChar('\"', result))) break;
            result = Parse(SymbolType_Spacing, result);
            break;
        case SymbolType_Literal_2_1:
            if (ParseChar('\"', _first)) break;
            result = Parse(SymbolType_Char, _first);
            break;

        //Class <- '[' (!']' Range)* ']' Spacing
        case SymbolType_Class:
            if (!(result = ParseChar('[', _first))) break;
            result = ParseZeroOrMore(SymbolType_Class_1, result);
            if (!(result = ParseChar(']', result))) break;
            result = Parse(SymbolType_Spacing, result);
            break;
        case SymbolType_Class_1:
            if (ParseChar(']', _first)) break;
            result = Parse(SymbolType_Range, _first);
            break;

        //Range <- Char ('-' Char)?
        case SymbolType_Range:
            if (!(result = Parse(SymbolType_Char, _first))) break;
            result = ParseOptional(SymbolType_Range_1, result);
            break;
        case SymbolType_Range_1:
            if (!(result = ParseChar('-', _first))) break;
            result = Parse(SymbolType_Char, result);
            break;

        //Char <- '\\' ([nrt'"\[\]\\] / [1-9][0-9]*)
        //      / !'\\' .
        case SymbolType_Char:
            if (result = Parse(SymbolType_Char_1, _first)) break;
            result = Parse(SymbolType_Char_2, _first);
            break;
        case SymbolType_Char_1:
            if (!(result = ParseChar('\\', _first))) break;
            result = Parse(SymbolType_Char_1_1, result);
            break;
        case SymbolType_Char_1_1:
            if (result = Parse(SymbolType_Char_1_1_1, _first)) break;
            result = Parse(SymbolType_Char_1_1_2, _first);
            break;
        case SymbolType_Char_1_1_1:
            if (result = ParseChar('n', _first)) break;
            if (result = ParseChar('r', _first)) break;
            if (result = ParseChar('t', _first)) break;
            if (result = ParseChar('\'', _first)) break;
            if (result = ParseChar('\"', _first)) break;
            if (result = ParseChar('[', _first)) break;
            if (result = ParseChar(']', _first)) break;
            result = ParseChar('\\', _first);
            break;
        case SymbolType_Char_1_1_2:
            if (!(result = ParseRange('1', '9', _first))) break;
            result = ParseOneOrMore(SymbolType_Char_1_1_2_1, result);
            break;
        case SymbolType_Char_1_1_2_1:
            result = ParseRange('0', '9', _first);
            break;
        case SymbolType_Char_2:
            if (ParseChar('\\', _first)) break;
            result = ParseAnyChar(_first);
            break;

        //LEFTARROW <- '<-' Spacing
        case SymbolType_LEFTARROW:
            if (!(result = ParseString("<-", _first))) break;
            result = Parse(SymbolType_Spacing, result);
            break;

        //SLASH <- '/' Spacing
        case SymbolType_SLASH:
            if (!(result = ParseChar('/', _first))) break;
            result = Parse(SymbolType_Spacing, result);
            break;

        //AND <- '&' Spacing
        case SymbolType_AND:
            if (!(result = ParseChar('&', _first))) break;
            result = Parse(SymbolType_Spacing, result);
            break;

        //NOT <- '!' Spacing
        case SymbolType_NOT:
            if (!(result = ParseChar('!', _first))) break;
            result = Parse(SymbolType_Spacing, result);
            break;

        //QUESTION <- '?' Spacing
        case SymbolType_QUESTION:
            if (!(result = ParseChar('?', _first))) break;
            result = Parse(SymbolType_Spacing, result);
            break;

        //STAR <- '*' Spacing
        case SymbolType_STAR:
            if (!(result = ParseChar('*', _first))) break;
            result = Parse(SymbolType_Spacing, result);
            break;

        //PLUS <- '+' Spacing
        case SymbolType_PLUS:
            if (!(result = ParseChar('+', _first))) break;
            result = Parse(SymbolType_Spacing, result);
            break;

        //OPEN <- '(' Spacing
        case SymbolType_OPEN:
            if (!(result = ParseChar('(', _first))) break;
            result = Parse(SymbolType_Spacing, result);
            break;

        //CLOSE <- ')' Spacing
        case SymbolType_CLOSE:
            if (!(result = ParseChar(')', _first))) break;
            result = Parse(SymbolType_Spacing, result);
            break;

        //DOT <- '.' Spacing
        case SymbolType_DOT:
            if (!(result = ParseChar('.', _first))) break;
            result = Parse(SymbolType_Spacing, result);
            break;

        //Spacing <- (Space / Comment)*
        case SymbolType_Spacing:
            result = ParseZeroOrMore(SymbolType_Spacing_1, _first);
            break;
        case SymbolType_Spacing_1:
            if (result = Parse(SymbolType_Space, _first)) break;
            result = Parse(SymbolType_Comment, _first);
            break;

        //Comment <- '#' (!EndOfLine .)* EndOfLine
        case SymbolType_Comment:
            if (!(result = ParseChar('#', _first))) break;
            result = ParseZeroOrMore(SymbolType_Comment_1, result);
            result = Parse(SymbolType_EndOfLine, result);
            break;
        case SymbolType_Comment_1:
            if (Parse(SymbolType_EndOfLine, _first)) break;
            result = ParseAnyChar(_first);
            break;

        //Space <- ' ' / '\t' / EndOfLine
        case SymbolType_Space:
            if (result = ParseChar(' ', _first)) break;
            if (result = ParseChar('\t', _first)) break;
            result = Parse(SymbolType_EndOfLine, _first);
            break;

        //EndOfLine <- '\r\n' / '\n' / '\r'
        case SymbolType_EndOfLine:
            if (result = ParseString("\r\n", _first)) break;
            if (result = ParseChar('\n', _first)) break;
            result = ParseChar('\r', _first);
            break;

        //EndOfFile <- !.
        case SymbolType_EndOfFile:
            if (ParseAnyChar(_first)) break;
            result = _first;
            break;
    }
    if (result)
        _first->result[_type] = result;
    return result;
}
