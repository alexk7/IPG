#include "Common.h"
#include "GenerateParser.h"
#include "Grammar.h"

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
		_os << "\t";
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
	bool mTraverse;
	
public:
	ParserGenerator(std::ostream& _source, const Grammar& _grammar,
									bool _traverse, int nextVarIndex)
	: mSource(_source)
	, mGrammar(_grammar)
	, mTabs(2)
	, mNextVarIndex(nextVarIndex)
	, mTraverse(_traverse)
	{
	}
	
	int Emit(int _firstIndex, int _resultIndex, const Expression& expr)
	{
		switch (expr.GetType())
		{
			case ExpressionType_Empty:
			{
				return _firstIndex;
			}
				
			case ExpressionType_Choice:
			{
				if (_resultIndex == _firstIndex)
					_resultIndex = -1;
				const Expressions& children = expr.GetChildren();
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
						mSource << mTabs << "p" << _resultIndex << " = p"
						<< tempIndex << ";\n";   
					mSource << mTabs << "if (!p" << _resultIndex << ")\n";
				}
				mSource << mTabs << "{\n";
				++mTabs;
				tempIndex = Emit(_firstIndex, _resultIndex, children[last]);
				if (_resultIndex != tempIndex)
					mSource << mTabs << "p" << _resultIndex << " = p"
					<< tempIndex << ";\n";   
				for (size_t i = last; i > 0; --i)
				{
					mSource << --mTabs << "}\n";
				}
				return _resultIndex;
			}
				
			case ExpressionType_Sequence:
			{
				const Expressions& children = expr.GetChildren();
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
					mSource << mTabs << "p" << _resultIndex << " = p"
					<< tempIndex << ";\n";   
				for (size_t i = last; i > 0; --i)
				{
					mSource << --mTabs << "}\n";
				}
				return _resultIndex;
			}
				
			case ExpressionType_And:
			{
				int tempIndex = Emit(_firstIndex, (_resultIndex == _firstIndex) ? -1 : _resultIndex, expr.GetChild());
				if (_resultIndex == -1)
					_resultIndex = tempIndex;
				mSource << mTabs << "p" << _resultIndex << " = p" << tempIndex
				<< " ? p" << _firstIndex << " : 0;\n";
				return _resultIndex;
			}
				
			case ExpressionType_Not:
			{
				int tempIndex = Emit(_firstIndex, (_resultIndex == _firstIndex) ? -1 : _resultIndex, expr.GetChild());
				if (_resultIndex == -1)
					_resultIndex = tempIndex;
				mSource << mTabs << "p" << _resultIndex << " = p" << tempIndex
				<< " ? 0 : p" << _firstIndex << ";\n";
				return _resultIndex;
			}
				
			case ExpressionType_Optional:
			{
				int tempIndex = Emit(_firstIndex, (_resultIndex == _firstIndex) ? -1 : _resultIndex, expr.GetChild());
				if (_resultIndex == -1)
					_resultIndex = tempIndex;
				mSource << mTabs << "p" << _resultIndex << " = p" << tempIndex
				<< " ? p" << tempIndex << " : p" << _firstIndex << ";\n";
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
					mSource << "p" << _resultIndex << " = p" << _firstIndex
					<< ";\n";
				}
				mSource << mTabs << "for (;;)\n";
				mSource << mTabs << "{\n";
				++mTabs;
				int tempIndex = Emit(_resultIndex, -1, expr.GetChild());
				mSource << mTabs << "if (!p" << tempIndex << ")\n";
				mSource << ++mTabs << "break;\n";
				mSource << --mTabs << "p" << _resultIndex << " = p" << tempIndex
				<< ";\n";
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
				mSource << "p" << _resultIndex << " = ";
				const std::string& nonTerminal = expr.GetNonTerminal();
				if (mTraverse)
				{
					const Def& def = *mGrammar.defs.find(nonTerminal);
					const DefValue& defval = def.second;
					if (defval.isNode)
					{
						mSource << "Visit(p" << _firstIndex << ", PTNodeType_" << nonTerminal << ", v);\n";
					}
					else if (!defval.isNodeRef && defval.isMemoized)
					{
						mSource << "GetEnd(p" << _firstIndex << ", PTNodeType_" << nonTerminal << ");\n";
					}
					else
					{
						mSource << "Traverse_" << nonTerminal << "(p" << _firstIndex << ", v);\n";
					}
				}
				else
				{
					mSource << "Parse_" << nonTerminal << "(p" << _firstIndex << ");\n";
				}
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
				EscapeChar c1(expr.GetFirst());
				EscapeChar c2(expr.GetLast());
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
				mSource << "p" << _resultIndex << " = ParseChar(\'" << EscapeChar(expr.GetChar()) << "\', p" << _firstIndex << ");\n";
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

static void GenerateSymbolParseFunctionDecl(std::ofstream& _source, Defs::const_iterator _iDef)
{
	_source << "\tPTNode* Parse_" << _iDef->first << "(PTNode* _p);\n";
}

static bool IsMemoized(Defs::const_iterator _iDef)
{
	return _iDef->second.isMemoized;
}

static void GenerateSymbolParseFunction(std::ostream& _os, const Grammar& _g, Defs::const_iterator _iDef)
{
	_os <<
	"\tPTNode* Parse_" << _iDef->first << "(PTNode* p0)\n"
	"\t{\n";
	
	bool isMemoized = IsMemoized(_iDef);
	if (isMemoized)
	{
		_os <<
		"\t\tPTNode* p1 = GetEnd(p0, PTNodeType_" << _iDef->first << ");\n"
		"\t\tif (p1)\n"
		"\t\t\treturn p1;\n";
	}
	
	ParserGenerator parserGenerator(_os, _g, false, isMemoized ? 2 : 1);
	int resultIndex = parserGenerator.Emit(0, isMemoized ? 1 : -1, _iDef->second);
	
	if (isMemoized)
		_os << "\t\tSetEnd(p0, PTNodeType_" << _iDef->first << ", p" << resultIndex << ");\n";
	
	_os <<
	"\t\treturn p" << resultIndex << ";\n"
	"\t}\n";
}

static void GenerateSymbolTraverseFunctionDecl(std::ofstream& _source, Defs::const_iterator _iDef)
{
	_source << "\tPTNode* Traverse_" << _iDef->first << "(PTNode* _p, PTNodeVisitor& _v);\n";
}

static void GenerateSymbolTraverseFunction(std::ostream& _os, const Grammar& _grammar, Defs::const_iterator _iDef)
{
	_os <<
	"\tPTNode* Traverse_" << _iDef->first << "(PTNode* p0, PTNodeVisitor& v)\n"
	"\t{\n";
	
	bool isMemoized = IsMemoized(_iDef);
	if (isMemoized)
	{
		_os <<
		"\t\tif (!GetEnd(p0, PTNodeType_" << _iDef->first << "))\n"
		"\t\t\treturn 0;\n";
	}
	
	ParserGenerator parserGenerator(_os, _grammar, true, 1);
	int resultIndex = parserGenerator.Emit(0, -1, _iDef->second);
	
	_os <<
	"\t\treturn p" << resultIndex << ";\n"
	"\t}\n";
}

void GenerateParserSource(std::string _ipgName, std::string _folder, std::string _name, const Grammar& _grammar)
{
	std::ofstream source((_folder + _name + ".cpp").c_str());
	
	source <<
	"#include \"" << _name << ".h\"\n"
	"\n"
	"namespace " << _name << "\n"
	"{\n"
	"namespace\n"
	"{\n"
	"\tPTNode* ParseRange(char _rangeBegin, char _rangeEnd, PTNode* _symbol)\n"
	"\t{\n"
	"\t\tif (_symbol->value < _rangeBegin)\n"
	"\t\t\treturn 0;\n"
	"\t\tif (_symbol->value > _rangeEnd)\n"
	"\t\t\treturn 0;\n"
	"\t\treturn ++_symbol;\n"
	"\t}\n"
	"\t\n"
	"\tPTNode* ParseChar(char _char, PTNode* _symbol)\n"
	"\t{\n"
	"\t\tif (_symbol->value != _char)\n"
	"\t\t\treturn 0;\n"
	"\t\treturn ++_symbol;\n"
	"\t}\n"
	"\n"
	"\tPTNode* ParseAnyChar(PTNode* _symbol)\n"
	"\t{\n"
	"\t\tif (_symbol->value == 0)\n"
	"\t\t\treturn 0;\n"
	"\t\treturn ++_symbol;\n"
	"\t}\n"
	"\n"
	"\tPTNode* GetEnd(const PTNode* _symbol, PTNodeType _type)\n"
	"\t{\n"
	"\t\tPTNodeTypeToPtr::const_iterator i = _symbol->end.find(_type);\n"
	"\t\tif (i == _symbol->end.end())\n"
	"\t\t\treturn 0;\n"
	"\t\treturn i->second;\n"
	"\t}\n"
	"\n"
	"\tvoid SetEnd(PTNode* _symbol, PTNodeType _type, PTNode* _end)\n"
	"\t{\n"
	"\t\t_symbol->end[_type] = _end;\n"
	"\t}\n"
	"\n"
	"\tPTNode* Visit(PTNode* _symbol, PTNodeType _type, PTNodeVisitor& _visitor)\n"
	"\t{\n"
	"\t\tPTNode* end = GetEnd(_symbol, _type);\n"
	"\t\tif (end)\n"
	"\t\t\t_visitor(_symbol, _type);\n"
	"\t\treturn end;\n"
	"\t}\n"
	"\n";
	
	Defs::const_iterator i, iEnd = _grammar.defs.end();
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
		GenerateSymbolParseFunctionDecl(source, i);
	
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
	{
		source <<	"\n";
		GenerateSymbolParseFunction(source, _grammar, i);
	}
	
	source <<	"\n";
	
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
		GenerateSymbolTraverseFunctionDecl(source, i);
	
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
	{
		source << "\n";
		GenerateSymbolTraverseFunction(source, _grammar, i);
	}
		
	source <<
	"}\n"
	"\n"
	"\tPTNode* Parse(PTNodeType _type, PTNode* _symbol)\n"
	"\t{\n"
	"\t\tswitch (_type)\n"
	"\t\t{\n";
	
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
		source << "\t\t\tcase PTNodeType_" << i->first << ": return Parse_" << i->first << "(_symbol);\n";
	
	source <<
	"\t\t}\n"
	"\t\treturn 0;\n"
	"\t}\n"
	"\n"
	"\tPTNode* Traverse(PTNodeType _type, PTNode* _symbol, PTNodeVisitor& _visitor)\n"
	"\t{\n"
	"\t\tswitch (_type)\n"
	"\t\t{\n";
	
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
		source << "\t\t\tcase PTNodeType_" << i->first << ": return Traverse_" << i->first << "(_symbol, _visitor);\n";
	
	source <<
	"\t\t}\n"
	"\t\treturn 0;\n"
	"\t}\n"
	"}\n";
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
	"#include <map>\n"
	"\n"
	"namespace " << _name << "\n"
	"{\n"
	"\tenum PTNodeType\n"
	"\t{\n";
	
	Defs::const_iterator i, iEnd = _grammar.defs.end();
	int value = 0;
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
		header << "\t\t\tPTNodeType_" << i->first << " = " << ++value << ",\n";
	
	header <<
	"\t};\n"
	"\n"
	"\tstruct PTNode;\n"
	"\ttypedef std::map<PTNodeType, PTNode*> PTNodeTypeToPtr;\n"
	"\n"
	"\tstruct PTNode\n"
	"\t{\n"
	"\t\tchar value;\n"
	"\t\tPTNodeTypeToPtr end;\n"
	"\t};\n"
	"\n"
	"\tstruct PTNodeVisitor\n"
	"\t{\n"
	"\t\tvirtual void operator()(PTNode* _symbol, PTNodeType _type) = 0;\n"
	"\t};\n"
	"\n"
	"\tPTNode* Parse(PTNodeType _type, PTNode* _symbol);\n"
	"\tPTNode* Traverse(PTNodeType _type, PTNode* _symbol, PTNodeVisitor& _visitor);\n"
	"}\n"
	"\n"
	"#endif\n";
}
