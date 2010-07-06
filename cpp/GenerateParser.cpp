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

class ParserGenerator
{
    std::ostream& mSource;
    const Grammar& mGrammar;
    Tabs mTabs;
    int mNextVarIndex;

public:
    ParserGenerator(std::ostream& _source, const Grammar& _grammar)
    : mSource(_source)
    , mGrammar(_grammar)
    , mTabs(1)
    , mNextVarIndex(2)
    {
    }
    
    int Emit(int _firstIndex, int _resultIndex, const Expression& _expression)
    {
        switch (_expression.GetType())
        {
            case ExpressionType_Empty:
            {
                return _firstIndex;
            }
                
            case ExpressionType_Choice:
            {
                if (_resultIndex == _firstIndex)
                    _resultIndex = -1;
                const Expressions& children = _expression.GetChildren();
                int tempIndex = Emit(_firstIndex, _resultIndex, children[0]);
                mSource << mTabs << "if (!p" << tempIndex << ")\n";
                _resultIndex = tempIndex;
                size_t last = children.size()-1;
                for (size_t i = 1; i < last; ++i)
                {
                    mSource << mTabs << "{\n";
                    ++mTabs;
                    tempIndex = Emit(_firstIndex, _resultIndex, children[i]);
                    if (_resultIndex != tempIndex)
                        mSource << mTabs << "p" << _resultIndex << " = p" << tempIndex << ";\n";   
                    mSource << mTabs << "if (!p" << _resultIndex << ")\n";
                }
                mSource << mTabs << "{\n";
                ++mTabs;
                tempIndex = Emit(_firstIndex, _resultIndex, children[last]);
                if (_resultIndex != tempIndex)
                    mSource << mTabs << "p" << _resultIndex << " = p" << tempIndex << ";\n";   
                for (size_t i = last; i > 0; --i)
                {
                    mSource << --mTabs << "}\n";
                }
                return _resultIndex;
            }
                
            case ExpressionType_Sequence:
            {
                const Expressions& children = _expression.GetChildren();
                int tempIndex = Emit(_firstIndex, _resultIndex, children[0]);
                mSource << mTabs << "if (p" << tempIndex << ")\n";
                _resultIndex = tempIndex;
                size_t last = children.size()-1;
                for (size_t i = 1; i < last; ++i)
                {
                    mSource << mTabs << "{\n";
                    ++mTabs;
                    tempIndex = Emit(tempIndex, _resultIndex, children[i]);
                    mSource << mTabs << "if (p" << tempIndex << ")\n";
                }
                mSource << mTabs << "{\n";
                ++mTabs;
                tempIndex = Emit(tempIndex, _resultIndex, children[last]);
                if (_resultIndex != tempIndex)
                    mSource << mTabs << "p" << _resultIndex << " = p" << tempIndex << ";\n";   
                for (size_t i = last; i > 0; --i)
                {
                    mSource << --mTabs << "}\n";
                }
                return _resultIndex;
            }
                
            case ExpressionType_And:
            {
                int tempIndex = Emit(_firstIndex, (_resultIndex == _firstIndex) ? -1 : _resultIndex, _expression.GetChild());
                if (_resultIndex == -1)
                    _resultIndex = tempIndex;
                mSource << mTabs << "p" << _resultIndex << " = p" << tempIndex << " ? p" << _firstIndex << " : 0;\n";
                return _resultIndex;
            }
                
            case ExpressionType_Not:
            {
                int tempIndex = Emit(_firstIndex, (_resultIndex == _firstIndex) ? -1 : _resultIndex, _expression.GetChild());
                if (_resultIndex == -1)
                    _resultIndex = tempIndex;
                mSource << mTabs << "p" << _resultIndex << " = p" << tempIndex << " ? 0 : p" << _firstIndex << ";\n";
                return _resultIndex;
            }
                
            case ExpressionType_Optional:
            {
                int tempIndex = Emit(_firstIndex, (_resultIndex == _firstIndex) ? -1 : _resultIndex, _expression.GetChild());
                if (_resultIndex == -1)
                    _resultIndex = tempIndex;
                mSource << mTabs << "p" << _resultIndex << " = p" << tempIndex << " ? p" << tempIndex << " : p" << _firstIndex << ";\n";
                return _resultIndex;
            }
                
            case ExpressionType_ZeroOrMore:
            {
                if (_resultIndex != _firstIndex)
                {
                    mSource << mTabs;
                    if (_resultIndex == -1)
                    {
                        _resultIndex = mNextVarIndex++;
                        mSource << "PTNode* ";
                    }
                    mSource << "p" << _resultIndex << " = p" << _firstIndex << ";\n";
                }
                mSource << mTabs << "for (;;)\n";
                mSource << mTabs << "{\n";
                ++mTabs;
                int tempIndex = Emit(_resultIndex, -1, _expression.GetChild());
                mSource << mTabs << "if (!p" << tempIndex << ")\n";
                mSource << ++mTabs << "break;\n";
                mSource << --mTabs << "p" << _resultIndex << " = p" << tempIndex << ";\n";
                mSource << --mTabs << "}\n";
                return _resultIndex;
            }
                
            case ExpressionType_NonTerminal:
            {
                mSource << mTabs;
                if (_resultIndex == -1)
                {
                    _resultIndex = mNextVarIndex++;
                    mSource << "PTNode* ";
                }
                mSource << "p" << _resultIndex << " = Parse_" << _expression.GetNonTerminal() << "(p" << _firstIndex << ");\n";
                return _resultIndex;
            }
                
            case ExpressionType_Range:
            {
                mSource << mTabs;
                if (_resultIndex == -1)
                {
                    _resultIndex = mNextVarIndex++;
                    mSource << "PTNode* ";
                }
                EscapeChar c1(_expression.GetFirst());
                EscapeChar c2(_expression.GetLast());
                mSource << "p" << _resultIndex << " = ParseRange(\'" << c1 << "\', \'" << c2 << "\', p" << _firstIndex << ");\n";
                return _resultIndex;
            }
                
            case ExpressionType_Char:
            {
                mSource << mTabs;
                if (_resultIndex == -1)
                {
                    _resultIndex = mNextVarIndex++;
                    mSource << "PTNode* ";
                }
                mSource << "p" << _resultIndex << " = ParseChar(\'" << EscapeChar(_expression.GetChar()) << "\', p" << _firstIndex << ");\n";
                return _resultIndex;
            }
                
            case ExpressionType_Dot:
            {
                mSource << mTabs;
                if (_resultIndex == -1)
                {
                    _resultIndex = mNextVarIndex++;
                    mSource << "PTNode* ";
                }
                mSource << "p" << _resultIndex << " = ParseAnyChar(p" << _firstIndex << ");\n";
                return _resultIndex;
            }
                
            default:
            {
                assert(false);
                return -1;
            }
        }
    }
};

static void GenerateSymbolParseFunction(std::ostream& _os, const Grammar& _grammar, Grammar::Defs::const_iterator _iDef)
{
	_os <<
		"\nPTNode* Parse_" << _iDef->first << "(PTNode* p0)\n"
		"{\n"
        "    PTNode* p1 = p0->end.Get(PTNodeType_" << _iDef->first << ");\n"
        "    if (p1)\n"
        "        return p1;\n";
    
    ParserGenerator parserGenerator(_os, _grammar);
    int resultIndex = parserGenerator.Emit(0, 1, _iDef->second);
    
    _os <<
        "    if (p" << resultIndex << ")\n"
        "        p0->end.Set(PTNodeType_" << _iDef->first << ", p" << resultIndex << ");\n"
        "    return p" << resultIndex << ";\n}\n";
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
        "Result::Result() : mpHead(0) {}\n"
        "\n"
        "Result::~Result()\n"
        "{\n"
        "    Clear();\n"
        "}\n"
        "\n"
        "void Result::Set(int _index, PTNode* _result)\n"
		"{\n"
        "    Node* pNewNode = new Node;\n"
        "    pNewNode->index = _index;\n"
        "    pNewNode->pNext = mpHead;\n"
        "    pNewNode->result = _result;\n"
        "    mpHead = pNewNode;\n"
		"}\n"
		"\n"
		"PTNode* Result::Get(int _index) const\n"
		"{\n"
        "    for (Node* pNode = mpHead; pNode; pNode = pNode->pNext)\n"
        "    {\n"
        "        if (pNode->index == _index)\n"
        "            return pNode->result;\n"
        "    }\n"
        "    return 0;\n"
		"}\n"
		"\n"
		"void Result::Clear()\n"
		"{\n"
        "    Node* pNode = mpHead;\n"
        "    while (pNode)\n"
        "    {\n"
        "        Node* pNext = pNode->pNext;\n"
        "        delete pNode;\n"
        "        pNode = pNext;\n"
        "    }\n"
        "    mpHead = 0;\n"
        "}\n"
		"\n"
		"static PTNode* ParseRange(char _rangeBegin, char _rangeEnd, PTNode* _symbol)\n"
		"{\n"
		"    if (_symbol->value < _rangeBegin)\n"
		"        return 0;\n"
		"    if (_symbol->value > _rangeEnd)\n"
		"        return 0;\n"
		"    return ++_symbol;\n"
		"}\n"
		"\n"
		"static PTNode* ParseChar(char _char, PTNode* _symbol)\n"
		"{\n"
		"    if (_symbol->value != _char)\n"
		"        return 0;\n"
		"    return ++_symbol;\n"
		"}\n"
		"\n"
		"static PTNode* ParseAnyChar(PTNode* _symbol)\n"
		"{\n"
		"    if (_symbol->value == 0)\n"
		"        return 0;\n"
		"    return ++_symbol;\n"
		"}\n";

	iEnd = _grammar.defs.end();
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
		GenerateSymbolParseFunction(source, _grammar, i);
}

static void GenerateSymbolParseFunctionDecl(std::ofstream& _source, Grammar::Defs::const_iterator _iDef)
{
	_source << "PTNode* Parse_" << _iDef->first << "(PTNode* _p);\n";
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
		"struct PTNode;\n"
		"\n"
		"class Result\n"
		"{\n"
		"public:\n"
        "    Result();\n"
        "    ~Result();\n"
		"    void Set(int _index, PTNode* _result);\n"
		"    PTNode* Get(int _index) const;\n"
		"    void Clear();\n"
		"\n"
		"private:\n"
        "    struct Node\n"
        "    {\n"
        "        int index;\n"
        "        Node* pNext;\n"
        "        PTNode* result;\n"
        "    };\n"
        "\n"
        "    Node* mpHead;\n"
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
