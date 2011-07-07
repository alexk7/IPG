#include "Common.h"
#include "GenerateParser.h"
#include "Grammar.h"

#include <ctemplate/template.h>
#include <json_spirit.h>

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
	, mTabs(3)
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
						mSource << mTabs << "p" << _resultIndex << " = p" << tempIndex << ";\n";   
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
					mSource << mTabs << "p" << _resultIndex << " = p"	<< tempIndex << ";\n";   
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

void GenerateParser(std::string _folder, std::string _name, const Grammar& _grammar)
{
	ctemplate::TemplateDictionary dict(_name);
	dict.SetValue("name", _name);

	Defs::const_iterator i, iEnd = _grammar.defs.end();
	long value = 0;
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
	{
		ctemplate::TemplateDictionary* pDef = dict.AddSectionDictionary("def");
		pDef->SetValue("name", i->first);
		pDef->SetIntValue("value", ++value);
		
		bool isMemoized = i->second.isMemoized;
		if (isMemoized)
			pDef->ShowSection("isMemoized");
		
		std::ostringstream parseCodeStream;
		ParserGenerator parserGenerator(parseCodeStream, _grammar, false, isMemoized ? 2 : 1);
		int parseResultIndex = parserGenerator.Emit(0, isMemoized ? 1 : -1, i->second);
		
		std::string parseCodeFilename = "parseCode_" + i->first;
		std::string parseCode = parseCodeStream.str();
		ctemplate::StringToTemplateCache(parseCodeFilename, parseCode, ctemplate::STRIP_BLANK_LINES);
		
		pDef->AddIncludeDictionary("parseCode")->SetFilename(parseCodeFilename);
		pDef->SetIntValue("parseResultIndex", parseResultIndex);
		
		std::ostringstream traverseCodeStream;
		ParserGenerator traverserGenerator(traverseCodeStream, _grammar, true, 1);
		int traverseResultIndex = traverserGenerator.Emit(0, -1, i->second);
		
		std::string traverseCodeFilename = "traverseCode_" + i->first;
		std::string traverseCode = traverseCodeStream.str();
		ctemplate::StringToTemplateCache(traverseCodeFilename, traverseCode, ctemplate::STRIP_BLANK_LINES);

		pDef->AddIncludeDictionary("traverseCode")->SetFilename(traverseCodeFilename);
		pDef->SetIntValue("traverseResultIndex", traverseResultIndex);
	}

	std::string sourceText;
	if (!ctemplate::ExpandTemplate("Parser.cpp.tpl", ctemplate::STRIP_BLANK_LINES, &dict, &sourceText))
		throw std::runtime_error("CTemplate Parser.cpp.tpl expansion failed!");
	
	std::ofstream sourceFile;
	sourceFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	std::string sourcePath = _folder + _name + ".cpp";
	sourceFile.open(sourcePath.c_str());
	sourceFile << sourceText;
	
	std::string headerText;
	if (!ctemplate::ExpandTemplate("Parser.h.tpl", ctemplate::STRIP_BLANK_LINES, &dict, &headerText))
		throw std::runtime_error("CTemplate Parser.h.tpl expansion failed!");
	
	std::ofstream headerFile;
	headerFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	headerFile.open((_folder + _name + ".h").c_str());
	headerFile << headerText;
}
