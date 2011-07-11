#include "Common.h"
#include "GenerateParser.h"
#include "Grammar.h"

#include <ctemplate/template.h>
#include <boost/format.hpp>

class Tabs
{
public:
	Tabs(size_t _tabLevel = 0) : mTabLevel(_tabLevel) {}
	Tabs& operator++() { ++mTabLevel; return *this; }
	Tabs& operator--() { --mTabLevel; return *this; }
	Tabs Next() const  { return Tabs(mTabLevel + 1); }
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
				const Expression::Group& group = expr.GetGroup();
				int tempIndex = Emit(_firstIndex, _resultIndex, group.first);
				If(Not(RValue(tempIndex)));
				_resultIndex = tempIndex;
				OpenBlock();
				tempIndex = Emit(_firstIndex, _resultIndex, group.second);
				Assign(_resultIndex, tempIndex);
				CloseBlock();
				return _resultIndex;
			}
				
			case ExpressionType_Sequence:
			{
				const Expression::Group& group = expr.GetGroup();
				int tempIndex = Emit(_firstIndex, _resultIndex, group.first);
				If(RValue(tempIndex));
				_resultIndex = tempIndex;
				OpenBlock();
				tempIndex = Emit(tempIndex, _resultIndex, group.second);
				Assign(_resultIndex, tempIndex);
				CloseBlock();
				return _resultIndex;
			}

			case ExpressionType_Not:
			{
				int tempIndex = Emit(_firstIndex, (_resultIndex == _firstIndex) ? -1 : _resultIndex, expr.GetChild());
				if (_resultIndex == -1)
					_resultIndex = tempIndex;
				mSource << mTabs << boost::format("%1% = %2% ? 0 : %3%;\n") % LValue(_resultIndex) % RValue(tempIndex) % RValue(_firstIndex);
				return _resultIndex;
			}
				
			case ExpressionType_ZeroOrMore:
			{
				Assign(_resultIndex, _firstIndex);
				mSource << mTabs << "for (;;)\n";
				OpenBlock();
				int tempIndex = Emit(_resultIndex, -1, expr.GetChild());
				If(Not(RValue(tempIndex)));
				mSource << mTabs.Next() << "break;\n";
				Assign(_resultIndex, tempIndex);
				CloseBlock();
				return _resultIndex;
			}
				
			case ExpressionType_NonTerminal:
			{
				const std::string& nonTerminal = expr.GetNonTerminal();
				if (mTraverse)
				{
					const Def& def = *mGrammar.defs.find(nonTerminal);
					const DefValue& defval = def.second;
					if (defval.isNode)
					{
						if (_resultIndex == _firstIndex)
							_resultIndex = -1;
						mSource << mTabs << boost::format("%1% = %2%->end.find(PTNodeType_%3%)->second;\n") % LValue(_resultIndex) % RValue(_firstIndex) % nonTerminal;
						mSource << mTabs << boost::format("if (%1%)\n") % RValue(_resultIndex);
						mSource << mTabs.Next() << boost::format("v.push_back(PTNodeChild(PTNodeType_%1%, %2%));\n") % nonTerminal % RValue(_firstIndex);
						return _resultIndex;
					}
					else if (defval.isNodeRef)
					{
						mSource << mTabs << boost::format("%1% = Traverse_%2%(%3%, v);\n") % LValue(_resultIndex) % nonTerminal % RValue(_firstIndex);
						return _resultIndex;
					}
				}
				mSource << mTabs << boost::format("%1% = Parse_%2%(%3%);\n") % LValue(_resultIndex) % nonTerminal % RValue(_firstIndex);
				return _resultIndex;
			}
				
			case ExpressionType_Range:
			{
				mSource << mTabs << boost::format("%1% = (%2% >= \'%3%\' && %2% <= \'%4%\') ? %5% : 0;\n")
					% LValue(_resultIndex)
					% Deref(_firstIndex)
					% EscapeChar(expr.GetFirst())
					% EscapeChar(expr.GetLast())
					% Next(_firstIndex);
				return _resultIndex;
			}
				
			case ExpressionType_Char:
			{
				mSource << mTabs << boost::format("%1% = (%2% == \'%3%\') ? %4% : 0;\n")
					% LValue(_resultIndex)
					% Deref(_firstIndex)
					% EscapeChar(expr.GetChar())
					% Next(_firstIndex);
				return _resultIndex;
			}
				
			case ExpressionType_Dot:
			{
				mSource << mTabs << boost::format("%1% = (%2% != 0) ? %3% : 0;\n")
					% LValue(_resultIndex)
					% Deref(_firstIndex)
					% Next(_firstIndex);
				return _resultIndex;
			}
				
			default:
			{
				assert(false);
				return -1;
			}
		}
	}
	
	void If(std::string _cond)
	{
		mSource << mTabs << boost::format("if (%1%)\n") % _cond;
	}
	
	void OpenBlock()
	{
		mSource << mTabs << "{\n";
		++mTabs;
	}
	
	void CloseBlock()
	{
		--mTabs;
		mSource << mTabs << "}\n";
	}
	
	void Assign(int& _toIndex, int _fromIndex)
	{
		if (_toIndex != _fromIndex)
			mSource << mTabs << boost::format("%1% = %2%;\n") % LValue(_toIndex) % RValue(_fromIndex); 
	}
	
	static std::string Next(int _index)
	{
		return RValue(_index) + "+1";
	}
	
	static std::string Deref(int _index)
	{
		return RValue(_index) + "->value";
	}
	
	static std::string Not(std::string _expr)
	{
		return "!" + _expr;
	}
	
	std::string LValue(int& _index)
	{
		if (_index == -1)
		{
			_index = mNextVarIndex++;
			return str(boost::format("Node* %1%") % RValue(_index));
		}
		else
		{
			return RValue(_index);
		}
	}
	
	static std::string RValue(int _index)
	{
		assert(_index != -1);
		return str(boost::format("p%1%") % _index);
	}
	
	static std::string EscapeChar(char _c)
	{
		switch (_c)
		{
			case '\\': return "\\\\";
			case '\n': return "\\n";
			case '\r': return "\\r";
			case '\t': return "\\t";
			case '\'': return "\\\'";
			case '\"': return "\\\"";
			default: return std::string(1, _c);
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
		ParserGenerator parserGenerator(parseCodeStream, _grammar, false, 1);
		int parseResultIndex = parserGenerator.Emit(0, -1, i->second);
		
		std::string parseCodeFilename = "parseCode_" + i->first;
		std::string parseCode = parseCodeStream.str();
		ctemplate::StringToTemplateCache(parseCodeFilename, parseCode, ctemplate::STRIP_BLANK_LINES);
		
		pDef->AddIncludeDictionary("parseCode")->SetFilename(parseCodeFilename);
		pDef->SetIntValue("parseResultIndex", parseResultIndex);
		
		std::ostringstream traverseCodeStream;
		ParserGenerator traverserGenerator(traverseCodeStream, _grammar, true, 2);
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
