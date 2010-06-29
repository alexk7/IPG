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

std::ostream& operator<<(std::ostream& _os, Tabs _tabs)
{
	size_t tabLevel = _tabs.GetTabLevel();
	for (size_t i = 0; i < tabLevel; ++i)
		_os << "    ";
	return _os;
}

static void EscapeChar(std::ostream& _os, char _char)
{
	switch (_char)
	{
		case '\\': _os << "\\\\";  break;
		case '\n': _os << "\\n";   break;
		case '\r': _os << "\\r";   break;
		case '\t': _os << "\\t";   break;
		case '\'': _os << "\\\'";   break;
		case '\"': _os << "\\\"";   break;
		default:   _os.put(_char); break;
	}
}

static void GenerateParseStatement(std::ofstream& _source, Tabs& _tabs, std::string _resultVar, const Expression& _expression, std::string _firstVar)
{
	ExpressionType type = _expression.GetType();
	assert(!IsGroup(type));
	
	switch (type)
	{
		case ExpressionType_Empty:
            if (_resultVar != _firstVar)
                _source << _tabs << _resultVar << " = " << _firstVar << ";\n";
			break;

		case ExpressionType_And:
			_source << _tabs << _resultVar << " = ParseAnd(Parse_" << _expression.GetChild().GetNonTerminal() << ", " << _firstVar << ");\n";
			break;

		case ExpressionType_Not:
			_source << _tabs << _resultVar << " = ParseNot(Parse_" << _expression.GetChild().GetNonTerminal() << ", " << _firstVar << ");\n";
			break;

		case ExpressionType_Optional:
			_source << _tabs << _resultVar << " = ParseOptional(Parse_" << _expression.GetChild().GetNonTerminal() << ", " << _firstVar << ");\n";
			break;

		case ExpressionType_ZeroOrMore:
			_source << _tabs << _resultVar << " = ParseZeroOrMore(Parse_" << _expression.GetChild().GetNonTerminal() << ", " << _firstVar << ");\n";
			break;

		case ExpressionType_NonTerminal:
			_source << _tabs << _resultVar << " = Parse_" << _expression.GetNonTerminal() << "(" << _firstVar << ");\n";
			break;

		case ExpressionType_Dot:
			_source << _tabs << _resultVar << " = ParseAnyChar(" << _firstVar << ");\n"; break;
			
		case ExpressionType_Char:
			_source << _tabs << _resultVar << " = ParseChar(\'";
			EscapeChar(_source, _expression.GetChar());
			_source << "\', " << _firstVar << ");\n";
			break;
			
		case ExpressionType_Range:
			_source << _tabs << _resultVar << " = ParseRange(\'";
			EscapeChar(_source, _expression.GetFirst());
			_source << "\', \'";
			EscapeChar(_source, _expression.GetLast());
			_source << "\', " << _firstVar << ");\n";
			break;
	}
}

static void GenerateSymbolParseFunctionDecl(std::ofstream& _source, Grammar::Defs::const_iterator _iDef)
{
	_source << "PTNodeItr Parse_" << _iDef->first << "(PTNodeItr _first);\n";
}

static std::string GetResultVar(int _resultIndex)
{
    std::ostringstream oss;
    oss << "r" << _resultIndex;
    return oss.str();
}

static void GenerateParseSequence(std::ofstream& _source, Tabs& _tabs, const Expressions& _children)
{
    std::string resultVar = GetResultVar(0);
	GenerateParseStatement(_source, _tabs, resultVar, _children[0], "_first");
	_source << _tabs << "if (" << resultVar << ")\n";
	size_t last = _children.size()-1;
	for (size_t i = 1; i < last; ++i)
	{
		_source << _tabs << "{\n";
		GenerateParseStatement(_source, ++_tabs, resultVar, _children[i], resultVar);
		_source << _tabs << "if (" << resultVar << ")\n";
	}
	_source << _tabs << "{\n";
	GenerateParseStatement(_source, ++_tabs, resultVar, _children[last], resultVar);
	for (size_t i = last; i > 0; --i)
	{
		_source << --_tabs << "}\n";
	}
}

static void GenerateParseChoice(std::ofstream& _source, Tabs& _tabs, const Expressions& _children)
{
    std::string resultVar = GetResultVar(0);
	size_t last = _children.size()-1;
	for (size_t i = 0; i < last; ++i, ++_tabs)
	{
		GenerateParseStatement(_source, _tabs, resultVar, _children[i], "_first");
		_source << _tabs << "if (!" << resultVar << ")\n";
		_source << _tabs << "{\n";
	}
	GenerateParseStatement(_source, _tabs, resultVar, _children[last], "_first");
	for (size_t i = last; i > 0; --i)
	{
		_source << --_tabs << "}\n";
	}
}

static void GenerateSymbolParseFunction(std::ofstream& _source, Grammar::Defs::const_iterator _iDef, bool _memoized)
{
	Tabs tabs(1);
    std::string resultVar = GetResultVar(0);
    
	_source <<
		"\nPTNodeItr Parse_" << _iDef->first << "(PTNodeItr _first)\n"
		"{\n"
		"    PTNodeItr " << resultVar << ";\n";
	
	if (_memoized)
	{
		_source <<
			"    " << resultVar << " = _first->end.Get(PTNodeType_" << _iDef->first << ");\n"
			"    if (" << resultVar << ")\n"
			"        return " << resultVar << ";\n";
	}

	const Expression& expression = _iDef->second;
	ExpressionType type = expression.GetType();
	if (IsGroup(type))
	{
		const Expressions& children = expression.GetChildren();
		assert(children.size() >= 2);

		if (type == ExpressionType_Sequence)
		{
			GenerateParseSequence(_source, tabs, children);
		}
		else if (type == ExpressionType_Choice)
		{
			GenerateParseChoice(_source, tabs, children);
		}
	}
	else
	{
		GenerateParseStatement(_source, tabs, resultVar, expression, "_first");
	}
	
	if (_memoized)
	{
		_source <<
			"    if (" << resultVar << ")\n"
			"        _first->end.Set(PTNodeType_" << _iDef->first << ", " << resultVar << ");\n";
	}
	
	_source << "    return " << resultVar << ";\n}\n";
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
		"static PTNodeItr ParseNot(ParseFn _parse, PTNodeItr _first)\n"
		"{\n"
		"    if (_parse(_first))\n"
		"        return NULL;\n"
		"    return _first;\n"
		"}\n"
		"\n"
		"static PTNodeItr ParseAnd(ParseFn _parse, PTNodeItr _first)\n"
		"{\n"
		"    if (_parse(_first))\n"
		"        return _first;\n"
		"    return NULL;\n"
		"}\n"
		"\n"
		"static PTNodeItr ParseZeroOrMore(ParseFn _parse, PTNodeItr _first)\n"
		"{\n"
		"    PTNodeItr result;\n"
		"    while (result = _parse(_first))\n"
		"        _first = result;\n"
		"    return _first;\n"
		"}\n"
		"\n"
		"static PTNodeItr ParseOptional(ParseFn _parse, PTNodeItr _first)\n"
		"{\n"
		"    PTNodeItr result;\n"
		"    if (result = _parse(_first))\n"
		"        return result;\n"
		"    return _first;\n"
		"}\n"
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
		"}\n"
		"\n";

	iEnd = _grammar.defs.end();
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
		GenerateSymbolParseFunction(source, i, true);

	iEnd = _grammar.subDefs.end();
	for (i = _grammar.subDefs.begin(); i != iEnd; ++i)
		GenerateSymbolParseFunction(source, i, false);
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
	
	iEnd = _grammar.subDefs.end();
	for (i = _grammar.subDefs.begin(); i != iEnd; ++i)
		GenerateSymbolParseFunctionDecl(header, i);
	
	header <<
		"\n"
		"#endif\n";
}
