#include "Common.h"
#include "GenerateParser.h"
#include "Grammar.h"

#include <ctemplate/template.h>

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
	
	void Emit(const Expression& expr)
	{
		switch (expr.GetType())
		{
			case ExpressionType_Empty:
			{
				break;
			}
				
			case ExpressionType_Choice:
			{
				int backtrackIndex = Assign(-1, 0);
				const Expression::Group* pGroup = &expr.GetGroup();
				Emit(pGroup->first);
				If(Not(RValue(0)));
				OpenBlock();
				int openCount = 1;
				while (pGroup->second.GetType() == ExpressionType_Choice)
				{
					pGroup = &pGroup->second.GetGroup();
					Assign(0, backtrackIndex);
					Emit(pGroup->first);
					If(Not(RValue(0)));
					OpenBlock();
					++openCount;
				}
				Assign(0, backtrackIndex);
				Emit(pGroup->second);
				while (openCount--)
					CloseBlock();
				break;
			}
				
			case ExpressionType_Sequence:
			{
				const Expression::Group& group = expr.GetGroup();
				Emit(group.first);
				If(RValue(0));
				OpenBlock();
				Emit(group.second);
				CloseBlock();
				break;
			}

			case ExpressionType_Not:
			{
				int backtrackIndex = Assign(-1, 0);
				Emit(expr.GetChild());
				mSource << mTabs.Next() << boost::format("%1% = %1% ? 0 : %2%;\n") % RValue(0) % RValue(backtrackIndex);
				break;
			}
				
			case ExpressionType_ZeroOrMore:
			{
				mSource << mTabs << "for (;;)\n";
				OpenBlock();
				int backtrackIndex = Assign(-1, 0);
				Emit(expr.GetChild());
				If(Not(RValue(0)));
				OpenBlock();
				Assign(0, backtrackIndex);
				mSource << mTabs << "break;\n";
				CloseBlock();
				CloseBlock();
				break;
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
						int backtrackIndex = Assign(-1, 0);
						mSource << mTabs << boost::format("%1% = %1%->end.find(PTNodeType_%2%)->second;\n") % RValue(0) % nonTerminal;
						mSource << mTabs << boost::format("if (%1%)\n") % RValue(0);
						mSource << mTabs.Next() << boost::format("v.push_back(PTNodeChild(PTNodeType_%1%, %2%));\n") % nonTerminal % RValue(backtrackIndex);
						break;
					}
					else if (defval.isNodeRef)
					{
						mSource << mTabs << boost::format("%1% = Traverse_%2%(%1%, v);\n") % RValue(0) % nonTerminal;
						break;
					}
				}
				mSource << mTabs << boost::format("%1% = Parse_%2%(%1%);\n") % RValue(0) % nonTerminal;
				break;
			}
				
			case ExpressionType_Range:
			{
				mSource << mTabs << boost::format("%1% = (%2% >= \'%3%\' && %2% <= \'%4%\') ? %5% : 0;\n")
					% RValue(0)
					% Deref(0)
					% EscapeChar(expr.GetFirst())
					% EscapeChar(expr.GetLast())
					% Next(0);
				break;
			}
				
			case ExpressionType_Char:
			{
				mSource << mTabs << boost::format("%1% = (%2% == \'%3%\') ? %4% : 0;\n")
					% RValue(0)
					% Deref(0)
					% EscapeChar(expr.GetChar())
					% Next(0);
				break;
			}
				
			case ExpressionType_Dot:
			{
				mSource << mTabs << boost::format("%1% = (%2% != 0) ? %3% : 0;\n")
					% RValue(0)
					% Deref(0)
					% Next(0);
				break;
			}
				
			default:
			{
				assert(false);
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
	
	int Assign(int _toIndex, int _fromIndex)
	{
		if (_toIndex != _fromIndex)
			mSource << mTabs << boost::format("%1% = %2%;\n") % LValue(_toIndex) % RValue(_fromIndex);
		return _toIndex;
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
		if (_index == 0)
			return "p";
		else
			return str(boost::format("p%1%") % _index);
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
		parserGenerator.Emit(i->second);
		
		std::string parseCodeFilename = "parseCode_" + i->first;
		std::string parseCode = parseCodeStream.str();
		ctemplate::StringToTemplateCache(parseCodeFilename, parseCode, ctemplate::STRIP_BLANK_LINES);
		
		pDef->AddIncludeDictionary("parseCode")->SetFilename(parseCodeFilename);
		pDef->SetIntValue("parseResultIndex", 0);
		
		std::ostringstream traverseCodeStream;
		ParserGenerator traverserGenerator(traverseCodeStream, _grammar, true, 2);
		traverserGenerator.Emit(i->second);
		
		std::string traverseCodeFilename = "traverseCode_" + i->first;
		std::string traverseCode = traverseCodeStream.str();
		ctemplate::StringToTemplateCache(traverseCodeFilename, traverseCode, ctemplate::STRIP_BLANK_LINES);

		pDef->AddIncludeDictionary("traverseCode")->SetFilename(traverseCodeFilename);
		pDef->SetIntValue("traverseResultIndex", 0);
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
