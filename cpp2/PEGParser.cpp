#include "PEGParser.h"

enum PTNodeType
{
    PTNodeType_AND,
    PTNodeType_CLOSE,
    PTNodeType_Char,
    PTNodeType_Class,
    PTNodeType_Comment,
    PTNodeType_DOT,
    PTNodeType_Definition,
    PTNodeType_EndOfFile,
    PTNodeType_EndOfLine,
    PTNodeType_Expression,
    PTNodeType_Grammar,
    PTNodeType_Identifier,
    PTNodeType_LEFTARROW,
    PTNodeType_Literal,
    PTNodeType_NOT,
    PTNodeType_OPEN,
    PTNodeType_PLUS,
    PTNodeType_Prefix,
    PTNodeType_Primary,
    PTNodeType_QUESTION,
    PTNodeType_Range,
    PTNodeType_SLASH,
    PTNodeType_STAR,
    PTNodeType_Sequence,
    PTNodeType_Space,
    PTNodeType_Spacing,
    PTNodeType_Suffix,
};

size_t Result::GetPos(int _index) const
{
    return (mBitset << (PTNodeType_Count - _index)).count();
}

void Result::Set(int _index, PTNodeItr _result)
{
    size_t pos = GetPos(_index);
    if (mBitset.test(_index))
    {
        mVector[pos] = _result;
    }
    else
    {
        mBitset.set(_index);
        mVector.insert(mVector.begin() + pos, _result);
    }
}

PTNodeItr Result::Get(int _index) const
{
    if (!mBitset.test(_index))
        return NULL;

    size_t pos = GetPos(_index);
    return mVector[pos];
}

void Result::Clear()
{
    mBitset.reset();
    mVector.clear();
}

typedef PTNodeItr (*ParseFn)(PTNodeItr _first);

static PTNodeItr ParseNot(ParseFn _parse, PTNodeItr _first)
{
    if (_parse(_first))
        return NULL;
    return _first;
}

static PTNodeItr ParseAnd(ParseFn _parse, PTNodeItr _first)
{
    if (_parse(_first))
        return _first;
    return NULL;
}

static PTNodeItr ParseZeroOrMore(ParseFn _parse, PTNodeItr _first)
{
    PTNodeItr result;
    while (result = _parse(_first))
        _first = result;
    return _first;
}

static PTNodeItr ParseOptional(ParseFn _parse, PTNodeItr _first)
{
    PTNodeItr result;
    if (result = _parse(_first))
        return result;
    return _first;
}

static PTNodeItr ParseRange(char _rangeBegin, char _rangeEnd, PTNodeItr _symbol)
{
    if (_symbol->value < _rangeBegin)
        return NULL;
    if (_symbol->value > _rangeEnd)
        return NULL;
    return ++_symbol;
}

static PTNodeItr ParseChar(char _char, PTNodeItr _symbol)
{
    if (_symbol->value != _char)
        return NULL;
    return ++_symbol;
}

static PTNodeItr ParseAnyChar(PTNodeItr _symbol)
{
    if (_symbol->value == 0)
        return NULL;
    return ++_symbol;
}


PTNodeItr Parse_AND(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_AND);
    if (result)
        return result;
    result = ParseChar('&', _first);
    if (result)
    {
        result = Parse_Spacing(result);
    }
    if (result)
        _first->end.Set(PTNodeType_AND, result);
    return result;
}

PTNodeItr Parse_CLOSE(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_CLOSE);
    if (result)
        return result;
    result = ParseChar(')', _first);
    if (result)
    {
        result = Parse_Spacing(result);
    }
    if (result)
        _first->end.Set(PTNodeType_CLOSE, result);
    return result;
}

PTNodeItr Parse_Char(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Char);
    if (result)
        return result;
    result = Parse_Char_1(_first);
    if (!result)
    {
        result = Parse_Char_2(_first);
    }
    if (result)
        _first->end.Set(PTNodeType_Char, result);
    return result;
}

PTNodeItr Parse_Class(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Class);
    if (result)
        return result;
    result = ParseChar('[', _first);
    if (result)
    {
        result = ParseZeroOrMore(Parse_Class_1, result);
        if (result)
        {
            result = ParseChar(']', result);
            if (result)
            {
                result = Parse_Spacing(result);
            }
        }
    }
    if (result)
        _first->end.Set(PTNodeType_Class, result);
    return result;
}

PTNodeItr Parse_Comment(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Comment);
    if (result)
        return result;
    result = ParseChar('#', _first);
    if (result)
    {
        result = ParseZeroOrMore(Parse_Comment_1, result);
        if (result)
        {
            result = Parse_EndOfLine(result);
        }
    }
    if (result)
        _first->end.Set(PTNodeType_Comment, result);
    return result;
}

PTNodeItr Parse_DOT(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_DOT);
    if (result)
        return result;
    result = ParseChar('.', _first);
    if (result)
    {
        result = Parse_Spacing(result);
    }
    if (result)
        _first->end.Set(PTNodeType_DOT, result);
    return result;
}

PTNodeItr Parse_Definition(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Definition);
    if (result)
        return result;
    result = Parse_Identifier(_first);
    if (result)
    {
        result = Parse_Spacing(result);
        if (result)
        {
            result = Parse_LEFTARROW(result);
            if (result)
            {
                result = Parse_Expression(result);
            }
        }
    }
    if (result)
        _first->end.Set(PTNodeType_Definition, result);
    return result;
}

PTNodeItr Parse_EndOfFile(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_EndOfFile);
    if (result)
        return result;
    result = ParseNot(Parse_EndOfFile_1, _first);
    if (result)
        _first->end.Set(PTNodeType_EndOfFile, result);
    return result;
}

PTNodeItr Parse_EndOfLine(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_EndOfLine);
    if (result)
        return result;
    result = Parse_EndOfLine_1(_first);
    if (!result)
    {
        result = ParseChar('\n', _first);
        if (!result)
        {
            result = ParseChar('\r', _first);
        }
    }
    if (result)
        _first->end.Set(PTNodeType_EndOfLine, result);
    return result;
}

PTNodeItr Parse_Expression(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Expression);
    if (result)
        return result;
    result = Parse_Sequence(_first);
    if (result)
    {
        result = ParseZeroOrMore(Parse_Expression_1, result);
    }
    if (result)
        _first->end.Set(PTNodeType_Expression, result);
    return result;
}

PTNodeItr Parse_Grammar(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Grammar);
    if (result)
        return result;
    result = Parse_Spacing(_first);
    if (result)
    {
        result = Parse_Definition(result);
        if (result)
        {
            result = ParseZeroOrMore(Parse_Grammar_1, result);
            if (result)
            {
                result = Parse_EndOfFile(result);
            }
        }
    }
    if (result)
        _first->end.Set(PTNodeType_Grammar, result);
    return result;
}

PTNodeItr Parse_Identifier(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Identifier);
    if (result)
        return result;
    result = Parse_Identifier_1(_first);
    if (result)
    {
        result = ParseZeroOrMore(Parse_Identifier_2, result);
    }
    if (result)
        _first->end.Set(PTNodeType_Identifier, result);
    return result;
}

PTNodeItr Parse_LEFTARROW(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_LEFTARROW);
    if (result)
        return result;
    result = ParseChar('<', _first);
    if (result)
    {
        result = ParseChar('-', result);
        if (result)
        {
            result = Parse_Spacing(result);
        }
    }
    if (result)
        _first->end.Set(PTNodeType_LEFTARROW, result);
    return result;
}

PTNodeItr Parse_Literal(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Literal);
    if (result)
        return result;
    result = Parse_Literal_1(_first);
    if (!result)
    {
        result = Parse_Literal_2(_first);
    }
    if (result)
        _first->end.Set(PTNodeType_Literal, result);
    return result;
}

PTNodeItr Parse_NOT(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_NOT);
    if (result)
        return result;
    result = ParseChar('!', _first);
    if (result)
    {
        result = Parse_Spacing(result);
    }
    if (result)
        _first->end.Set(PTNodeType_NOT, result);
    return result;
}

PTNodeItr Parse_OPEN(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_OPEN);
    if (result)
        return result;
    result = ParseChar('(', _first);
    if (result)
    {
        result = Parse_Spacing(result);
    }
    if (result)
        _first->end.Set(PTNodeType_OPEN, result);
    return result;
}

PTNodeItr Parse_PLUS(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_PLUS);
    if (result)
        return result;
    result = ParseChar('+', _first);
    if (result)
    {
        result = Parse_Spacing(result);
    }
    if (result)
        _first->end.Set(PTNodeType_PLUS, result);
    return result;
}

PTNodeItr Parse_Prefix(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Prefix);
    if (result)
        return result;
    result = ParseOptional(Parse_Prefix_1, _first);
    if (result)
    {
        result = Parse_Suffix(result);
    }
    if (result)
        _first->end.Set(PTNodeType_Prefix, result);
    return result;
}

PTNodeItr Parse_Primary(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Primary);
    if (result)
        return result;
    result = Parse_Primary_1(_first);
    if (!result)
    {
        result = Parse_Primary_2(_first);
        if (!result)
        {
            result = Parse_Literal(_first);
            if (!result)
            {
                result = Parse_Class(_first);
                if (!result)
                {
                    result = Parse_DOT(_first);
                }
            }
        }
    }
    if (result)
        _first->end.Set(PTNodeType_Primary, result);
    return result;
}

PTNodeItr Parse_QUESTION(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_QUESTION);
    if (result)
        return result;
    result = ParseChar('?', _first);
    if (result)
    {
        result = Parse_Spacing(result);
    }
    if (result)
        _first->end.Set(PTNodeType_QUESTION, result);
    return result;
}

PTNodeItr Parse_Range(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Range);
    if (result)
        return result;
    result = Parse_Char(_first);
    if (result)
    {
        result = ParseOptional(Parse_Range_1, result);
    }
    if (result)
        _first->end.Set(PTNodeType_Range, result);
    return result;
}

PTNodeItr Parse_SLASH(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_SLASH);
    if (result)
        return result;
    result = ParseChar('/', _first);
    if (result)
    {
        result = Parse_Spacing(result);
    }
    if (result)
        _first->end.Set(PTNodeType_SLASH, result);
    return result;
}

PTNodeItr Parse_STAR(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_STAR);
    if (result)
        return result;
    result = ParseChar('*', _first);
    if (result)
    {
        result = Parse_Spacing(result);
    }
    if (result)
        _first->end.Set(PTNodeType_STAR, result);
    return result;
}

PTNodeItr Parse_Sequence(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Sequence);
    if (result)
        return result;
    result = ParseZeroOrMore(Parse_Sequence_1, _first);
    if (result)
        _first->end.Set(PTNodeType_Sequence, result);
    return result;
}

PTNodeItr Parse_Space(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Space);
    if (result)
        return result;
    result = ParseChar(' ', _first);
    if (!result)
    {
        result = ParseChar('\t', _first);
        if (!result)
        {
            result = Parse_EndOfLine(_first);
        }
    }
    if (result)
        _first->end.Set(PTNodeType_Space, result);
    return result;
}

PTNodeItr Parse_Spacing(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Spacing);
    if (result)
        return result;
    result = ParseZeroOrMore(Parse_Spacing_1, _first);
    if (result)
        _first->end.Set(PTNodeType_Spacing, result);
    return result;
}

PTNodeItr Parse_Suffix(PTNodeItr _first)
{
    PTNodeItr result;
    result = _first->end.Get(PTNodeType_Suffix);
    if (result)
        return result;
    result = Parse_Primary(_first);
    if (result)
    {
        result = ParseOptional(Parse_Suffix_1, result);
    }
    if (result)
        _first->end.Set(PTNodeType_Suffix, result);
    return result;
}

PTNodeItr Parse_Char_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseChar('\\', _first);
    if (result)
    {
        result = Parse_Char_1_1(result);
    }
    return result;
}

PTNodeItr Parse_Char_1_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseChar('n', _first);
    if (!result)
    {
        result = ParseChar('r', _first);
        if (!result)
        {
            result = ParseChar('t', _first);
            if (!result)
            {
                result = ParseChar('\'', _first);
                if (!result)
                {
                    result = ParseChar('\"', _first);
                    if (!result)
                    {
                        result = ParseChar('[', _first);
                        if (!result)
                        {
                            result = ParseChar(']', _first);
                            if (!result)
                            {
                                result = ParseChar('\\', _first);
                                if (!result)
                                {
                                    result = Parse_Char_1_1_1(_first);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

PTNodeItr Parse_Char_1_1_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseRange('1', '9', _first);
    if (result)
    {
        result = ParseZeroOrMore(Parse_Char_1_1_1_1, result);
    }
    return result;
}

PTNodeItr Parse_Char_1_1_1_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseRange('0', '9', _first);
    return result;
}

PTNodeItr Parse_Char_2(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseNot(Parse_Char_2_1, _first);
    if (result)
    {
        result = ParseAnyChar(result);
    }
    return result;
}

PTNodeItr Parse_Char_2_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseChar('\\', _first);
    return result;
}

PTNodeItr Parse_Class_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseNot(Parse_Class_1_1, _first);
    if (result)
    {
        result = Parse_Range(result);
    }
    return result;
}

PTNodeItr Parse_Class_1_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseChar(']', _first);
    return result;
}

PTNodeItr Parse_Comment_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseNot(Parse_Comment_1_1, _first);
    if (result)
    {
        result = ParseAnyChar(result);
    }
    return result;
}

PTNodeItr Parse_Comment_1_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = Parse_EndOfLine(_first);
    return result;
}

PTNodeItr Parse_EndOfFile_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseAnyChar(_first);
    return result;
}

PTNodeItr Parse_EndOfLine_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseChar('\r', _first);
    if (result)
    {
        result = ParseChar('\n', result);
    }
    return result;
}

PTNodeItr Parse_Expression_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = Parse_SLASH(_first);
    if (result)
    {
        result = Parse_Sequence(result);
    }
    return result;
}

PTNodeItr Parse_Grammar_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = Parse_Definition(_first);
    return result;
}

PTNodeItr Parse_Identifier_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseRange('a', 'z', _first);
    if (!result)
    {
        result = ParseRange('A', 'Z', _first);
        if (!result)
        {
            result = ParseChar('_', _first);
        }
    }
    return result;
}

PTNodeItr Parse_Identifier_2(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseRange('a', 'z', _first);
    if (!result)
    {
        result = ParseRange('A', 'Z', _first);
        if (!result)
        {
            result = ParseRange('0', '9', _first);
            if (!result)
            {
                result = ParseChar('_', _first);
            }
        }
    }
    return result;
}

PTNodeItr Parse_Literal_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseChar('\'', _first);
    if (result)
    {
        result = ParseZeroOrMore(Parse_Literal_1_1, result);
        if (result)
        {
            result = ParseChar('\'', result);
            if (result)
            {
                result = Parse_Spacing(result);
            }
        }
    }
    return result;
}

PTNodeItr Parse_Literal_1_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseNot(Parse_Literal_1_1_1, _first);
    if (result)
    {
        result = Parse_Char(result);
    }
    return result;
}

PTNodeItr Parse_Literal_1_1_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseChar('\'', _first);
    return result;
}

PTNodeItr Parse_Literal_2(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseChar('\"', _first);
    if (result)
    {
        result = ParseZeroOrMore(Parse_Literal_2_1, result);
        if (result)
        {
            result = ParseChar('\"', result);
            if (result)
            {
                result = Parse_Spacing(result);
            }
        }
    }
    return result;
}

PTNodeItr Parse_Literal_2_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseNot(Parse_Literal_2_1_1, _first);
    if (result)
    {
        result = Parse_Char(result);
    }
    return result;
}

PTNodeItr Parse_Literal_2_1_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseChar('\"', _first);
    return result;
}

PTNodeItr Parse_Prefix_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = Parse_AND(_first);
    if (!result)
    {
        result = Parse_NOT(_first);
    }
    return result;
}

PTNodeItr Parse_Primary_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = Parse_Identifier(_first);
    if (result)
    {
        result = Parse_Spacing(result);
        if (result)
        {
            result = ParseNot(Parse_Primary_1_1, result);
        }
    }
    return result;
}

PTNodeItr Parse_Primary_1_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = Parse_LEFTARROW(_first);
    return result;
}

PTNodeItr Parse_Primary_2(PTNodeItr _first)
{
    PTNodeItr result;
    result = Parse_OPEN(_first);
    if (result)
    {
        result = Parse_Expression(result);
        if (result)
        {
            result = Parse_CLOSE(result);
        }
    }
    return result;
}

PTNodeItr Parse_Range_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = ParseChar('-', _first);
    if (result)
    {
        result = Parse_Char(result);
    }
    return result;
}

PTNodeItr Parse_Sequence_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = Parse_Prefix(_first);
    return result;
}

PTNodeItr Parse_Spacing_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = Parse_Space(_first);
    if (!result)
    {
        result = Parse_Comment(_first);
    }
    return result;
}

PTNodeItr Parse_Suffix_1(PTNodeItr _first)
{
    PTNodeItr result;
    result = Parse_QUESTION(_first);
    if (!result)
    {
        result = Parse_STAR(_first);
        if (!result)
        {
            result = Parse_PLUS(_first);
        }
    }
    return result;
}
