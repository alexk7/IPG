/*
 * GenerateParser.cpp
 *
 *  Created on: 7-12-2009
 *      Author: alexk7
 */

#include "Common.h"
#include "GenerateParser.h"
#include "AST/Grammar.h"

class Tabs
{
public:
	Tabs(size_t _tabLevel = 0) : mTabLevel(_tabLevel) {}
	Tabs& operator++() { ++mTabLevel; return *this; }
	Tabs& operator--() { --mTabLevel; return *this; }
	size_t GetTabLevel() const { return mTabLevel; }

private:
	size_t mTabLevel;
};

static std::ostream& operator<<(std::ostream& _os, Tabs _tabs)
{
	size_t tabLevel = _tabs.GetTabLevel();
	for (size_t i = 0; i < tabLevel; ++i)
		_os << "    ";
	return _os;
}

struct EscapeChar
{
    EscapeChar(char _c) : c(_c) {}
    char c;
};

static std::ostream& operator<<(std::ostream& _os, EscapeChar _escapeChar)
{
	switch (_escapeChar.c)
	{
		case '\\': _os << "\\\\"; break;
		case '\n': _os << "\\n";  break;
		case '\r': _os << "\\r";  break;
		case '\t': _os << "\\t";  break;
		case '\'': _os << "\\\'"; break;
		case '\"': _os << "\\\""; break;
		default: _os.put(_escapeChar.c);
	}
    return _os;
}

static int GenerateParseStatement(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression);

static int GenerateParseEmpty(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    return _firstIndex;
}

static int GenerateParseAnd(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    int tempIndex = GenerateParseStatement(_os, _tabs, _nextVarIndex, _firstIndex, (_resultIndex == _firstIndex) ? -1 : _resultIndex, _expression.GetChild());
    if (_resultIndex == -1)
        _resultIndex = tempIndex;
    _os << _tabs << "i" << _resultIndex << " = i" << tempIndex << " ? i" << _firstIndex << " : NULL;\n";
    return _resultIndex;
}

static int GenerateParseNot(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    int tempIndex = GenerateParseStatement(_os, _tabs, _nextVarIndex, _firstIndex, (_resultIndex == _firstIndex) ? -1 : _resultIndex, _expression.GetChild());
    if (_resultIndex == -1)
        _resultIndex = tempIndex;
    _os << _tabs << "i" << _resultIndex << " = i" << tempIndex << " ? NULL : i" << _firstIndex << ";\n";
    return _resultIndex;
}

static int GenerateParseOptional(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    int tempIndex = GenerateParseStatement(_os, _tabs, _nextVarIndex, _firstIndex, (_resultIndex == _firstIndex) ? -1 : _resultIndex, _expression.GetChild());
    if (_resultIndex == -1)
        _resultIndex = tempIndex;
    _os << _tabs << "i" << _resultIndex << " = i" << tempIndex << " ? i" << tempIndex << " : i" << _firstIndex << ";\n";
    return _resultIndex;
}

static int GenerateParseZeroOrMore(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    if (_resultIndex != _firstIndex)
    {
        _os << _tabs;
        if (_resultIndex == -1)
        {
            _resultIndex = _nextVarIndex++;
            _os << "PTNodeItr ";
        }
        _os << "i" << _resultIndex << " = i" << _firstIndex << ";\n";
    }
    _os << _tabs << "for (;;)\n";
    _os << _tabs << "{\n";
    int tempIndex = GenerateParseStatement(_os, ++_tabs, _nextVarIndex, _resultIndex, -1, _expression.GetChild());
    _os << _tabs << "if (!i" << tempIndex << ")\n";
    _os << ++_tabs << "break;\n";
    _os << --_tabs << "i" << _resultIndex << " = i" << tempIndex << ";\n";
    _os << --_tabs << "}\n";
    return _resultIndex;
}

static int GenerateParseNonTerminal(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    _os << _tabs;
    if (_resultIndex == -1)
    {
        _resultIndex = _nextVarIndex++;
        _os << "PTNodeItr ";
    }
    _os << "i" << _resultIndex << " = Parse_" << _expression.GetNonTerminal() << "(i" << _firstIndex << ");\n";
    return _resultIndex;
}

static int GenerateParseDot(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    _os << _tabs;
    if (_resultIndex == -1)
    {
        _resultIndex = _nextVarIndex++;
        _os << "PTNodeItr ";
    }
    _os << "i" << _resultIndex << " = ParseAnyChar(i" << _firstIndex << ");\n";
    return _resultIndex;
}

static int GenerateParseChar(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    _os << _tabs;
    if (_resultIndex == -1)
    {
        _resultIndex = _nextVarIndex++;
        _os << "PTNodeItr ";
    }
    _os << "i" << _resultIndex << " = ParseChar(\'" << EscapeChar(_expression.GetChar()) << "\', i" << _firstIndex << ");\n";
    return _resultIndex;
}

static int GenerateParseRange(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    _os << _tabs;
    if (_resultIndex == -1)
    {
        _resultIndex = _nextVarIndex++;
        _os << "PTNodeItr ";
    }
    EscapeChar c1(_expression.GetFirst());
    EscapeChar c2(_expression.GetLast());
    _os << "i" << _resultIndex << " = ParseRange(\'" << c1 << "\', \'" << c2 << "\', i" << _firstIndex << ");\n";
    return _resultIndex;
}

static int GenerateParseSequence(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    const Expressions& children = _expression.GetChildren();
    int tempIndex = GenerateParseStatement(_os, _tabs, _nextVarIndex, _firstIndex, _resultIndex, children[0]);
	_os << _tabs << "if (i" << tempIndex << ")\n";
    _resultIndex = tempIndex;
	size_t last = children.size()-1;
	for (size_t i = 1; i < last; ++i)
	{
		_os << _tabs << "{\n";
		tempIndex = GenerateParseStatement(_os, ++_tabs, _nextVarIndex, tempIndex, _resultIndex, children[i]);
		_os << _tabs << "if (i" << tempIndex << ")\n";
	}
	_os << _tabs << "{\n";
	tempIndex = GenerateParseStatement(_os, ++_tabs, _nextVarIndex, tempIndex, _resultIndex, children[last]);
    if (_resultIndex != tempIndex)
        _os << _tabs << "i" << _resultIndex << " = i" << tempIndex << ";\n";   
	for (size_t i = last; i > 0; --i)
	{
		_os << --_tabs << "}\n";
	}
    return _resultIndex;
}

static int GenerateParseChoice(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    if (_resultIndex == _firstIndex)
        _resultIndex = -1;
    const Expressions& children = _expression.GetChildren();
    int tempIndex = GenerateParseStatement(_os, _tabs, _nextVarIndex, _firstIndex, _resultIndex, children[0]);
    _os << _tabs << "if (!i" << tempIndex << ")\n";
    _resultIndex = tempIndex;
	size_t last = children.size()-1;
	for (size_t i = 1; i < last; ++i)
	{
		_os << _tabs << "{\n";
		tempIndex = GenerateParseStatement(_os, ++_tabs, _nextVarIndex, _firstIndex, _resultIndex, children[i]);
        if (_resultIndex != tempIndex)
            _os << _tabs << "i" << _resultIndex << " = i" << tempIndex << ";\n";   
		_os << _tabs << "if (!i" << _resultIndex << ")\n";
	}
	_os << _tabs << "{\n";
	tempIndex = GenerateParseStatement(_os, ++_tabs, _nextVarIndex, _firstIndex, _resultIndex, children[last]);
    if (_resultIndex != tempIndex)
        _os << _tabs << "i" << _resultIndex << " = i" << tempIndex << ";\n";   
	for (size_t i = last; i > 0; --i)
	{
		_os << --_tabs << "}\n";
	}
    return _resultIndex;
}

typedef int (*GenerateParseExpressionFn)(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression);
static const GenerateParseExpressionFn generateParseExpressionFnTable[ExpressionType_Count] = {
    GenerateParseEmpty,
    GenerateParseChoice,
    GenerateParseSequence,
    GenerateParseAnd,
    GenerateParseNot,
    GenerateParseOptional,
    GenerateParseZeroOrMore,
    GenerateParseNonTerminal,
    GenerateParseRange,
    GenerateParseChar,
    GenerateParseDot
};

static int GenerateParseStatement(std::ostream& _os, Tabs& _tabs, int& _nextVarIndex, int _firstIndex, int _resultIndex, const Expression& _expression)
{
    return generateParseExpressionFnTable[_expression.GetType()](_os, _tabs, _nextVarIndex, _firstIndex, _resultIndex, _expression);
}

static void GenerateSymbolParseFunction(std::ostream& _os, Grammar::Defs::const_iterator _iDef, bool _memoized)
{
	Tabs tabs(1);
    
	_os <<
		"\nPTNodeItr Parse_" << _iDef->first << "(PTNodeItr i0)\n"
		"{\n";
    
	if (_memoized)
	{
		_os <<
            "    PTNodeItr i1 = i0->end.Get(PTNodeType_" << _iDef->first << ");\n"
			"    if (i1)\n"
			"        return i1;\n";
	}
    
    int nextVarIndex = 2;
    int resultIndex = GenerateParseStatement(_os, tabs, nextVarIndex, 0, 1, _iDef->second);
	if (_memoized)
	{
		_os <<
			"    if (i" << resultIndex << ")\n"
			"        i0->end.Set(PTNodeType_" << _iDef->first << ", i" << resultIndex << ");\n";
	}
	
	_os << "    return i" << resultIndex << ";\n}\n";
}

void GenerateParserSource(std::string _folder, std::string _name, const Grammar& _grammar)
{
	std::ofstream source((_folder + _name + ".cpp").c_str());
	source <<
		"#include \"" << _name << ".h\"\n"
		"\n"
		"enum PTNodeType\n"
		"{\n";
		
	Grammar::Defs::const_iterator i, iEnd = _grammar.defs.end();
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
		source << "    PTNodeType_" << i->first << ",\n";
	
	source <<
		"};\n"
		"\n"
		"size_t Result::GetPos(int _index) const\n"
		"{\n"
		"    return (mBitset << (PTNodeType_Count - _index)).count();\n"
		"}\n"
		"\n"
		"void Result::Set(int _index, PTNodeItr _result)\n"
		"{\n"
		"    size_t pos = GetPos(_index);\n"
		"    if (mBitset.test(_index))\n"
		"    {\n"
		"        mVector[pos] = _result;\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        mBitset.set(_index);\n"
		"        mVector.insert(mVector.begin() + pos, _result);\n"
		"    }\n"
		"}\n"
		"\n"
		"PTNodeItr Result::Get(int _index) const\n"
		"{\n"
		"    if (!mBitset.test(_index))\n"
		"        return NULL;\n"
		"\n"
		"    size_t pos = GetPos(_index);\n"
		"    return mVector[pos];\n"
		"}\n"
		"\n"
		"void Result::Clear()\n"
		"{\n"
		"    mBitset.reset();\n"
		"    mVector.clear();\n"
		"}\n"
		"\n"
		"typedef PTNodeItr (*ParseFn)(PTNodeItr _first);\n"
		"\n"
		"static PTNodeItr ParseRange(char _rangeBegin, char _rangeEnd, PTNodeItr _symbol)\n"
		"{\n"
		"    if (_symbol->value < _rangeBegin)\n"
		"        return NULL;\n"
		"    if (_symbol->value > _rangeEnd)\n"
		"        return NULL;\n"
		"    return ++_symbol;\n"
		"}\n"
		"\n"
		"static PTNodeItr ParseChar(char _char, PTNodeItr _symbol)\n"
		"{\n"
		"    if (_symbol->value != _char)\n"
		"        return NULL;\n"
		"    return ++_symbol;\n"
		"}\n"
		"\n"
		"static PTNodeItr ParseAnyChar(PTNodeItr _symbol)\n"
		"{\n"
		"    if (_symbol->value == 0)\n"
		"        return NULL;\n"
		"    return ++_symbol;\n"
		"}\n";

	iEnd = _grammar.defs.end();
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
		GenerateSymbolParseFunction(source, i, true);
}

static void GenerateSymbolParseFunctionDecl(std::ofstream& _source, Grammar::Defs::const_iterator _iDef)
{
	_source << "PTNodeItr Parse_" << _iDef->first << "(PTNodeItr i0);\n";
}

void GenerateParserHeader(std::string _folder, std::string _name, const Grammar& _grammar)
{
	std::ofstream header((_folder + _name + ".h").c_str());
	
	std::string includeGuard;
	std::transform(_name.begin(), _name.end(), std::back_inserter(includeGuard), toupper);
	includeGuard += "_H";
	
	header <<
		"#ifndef " << includeGuard << "\n"
		"#define " << includeGuard << "\n"
		"\n"
		"#include <bitset>\n"
		"#include <vector>\n"
		"\n"
		"struct PTNode;\n"
		"typedef PTNode* PTNodeItr;\n"
		"\n"
		"class Result\n"
		"{\n"
		"public:\n"
		"    void Set(int _index, PTNodeItr _result);\n"
		"    PTNodeItr Get(int _index) const;\n"
		"    void Clear();\n"
		"\n"
		"private:\n"
		"    size_t GetPos(int _index) const;\n"
		"    enum { PTNodeType_Count = " << _grammar.defs.size() << " };\n"
		"    std::bitset<PTNodeType_Count> mBitset;\n"
		"    std::vector<PTNodeItr> mVector;\n"
		"};\n"
		"\n"
		"struct PTNode\n"
		"{\n"
		"    char value;\n"
		"    Result end;\n"
		"};\n"
		"\n";
	
	Grammar::Defs::const_iterator i, iEnd = _grammar.defs.end();
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
		GenerateSymbolParseFunctionDecl(header, i);
	
	header <<
		"\n"
		"#endif\n";
}
