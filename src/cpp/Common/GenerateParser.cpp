#include "Common.h"
#include "GenerateParser.h"
#include "Grammar.h"

#include <ctemplate/template.h>

class ParserGenerator
{
	std::ostream& mSource;
	const Grammar& mGrammar;
	Tabs mTabs;
	int mNextBacktrackIndex;
	bool mTraverse;
	
public:
	ParserGenerator(std::ostream& _source, const Grammar& _grammar, bool _traverse = false)
	: mSource(_source)
	, mGrammar(_grammar)
	, mTabs(3)
	, mNextBacktrackIndex(1)
	, mTraverse(_traverse)
	{
	}
	
	void Emit(const Expression& expr, int _backtrackIndex = -1)
	{
		switch (expr.GetType())
		{
			case ExpressionType_Empty:
			{
				break;
			}
				
			case ExpressionType_Choice:
			{
				const Expression::Group& group = expr.GetGroup();
				bool visit = !group.first.isLeaf;
				DefineBacktrack(_backtrackIndex, visit);
				Emit(group.first, _backtrackIndex);
				If("!p");
				OpenBlock();
				Backtrack(_backtrackIndex, visit);
				Emit(group.second, _backtrackIndex);
				CloseBlock();
				break;
			}
				
			case ExpressionType_Sequence:
			{
				const Expression::Group& group = expr.GetGroup();
				Emit(group.first, _backtrackIndex);
				If("p");
				OpenBlock();
				Emit(group.second);
				CloseBlock();
				break;
			}

			case ExpressionType_Not:
			{
				bool traverse = mTraverse;
				mTraverse = false;
				DefineBacktrack(_backtrackIndex, false);
				Emit(expr.GetChild(), _backtrackIndex);
				mSource << mTabs << boost::format("if (p) p = 0; else p = %1%;\n") % BacktrackVar(_backtrackIndex);
				mTraverse = traverse;
				break;
			}
				
			case ExpressionType_ZeroOrMore:
			{
				mSource << mTabs << "for (;;)\n";
				OpenBlock();
				const Expression& child = expr.GetChild();
				bool visit = !child.isLeaf;
				DefineBacktrack(_backtrackIndex, visit);
				Emit(child, _backtrackIndex);
				If("!p");
				OpenBlock();
				Backtrack(_backtrackIndex, visit);
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
						mSource << mTabs << boost::format("p = ::Visit(PTNodeType_%1%, p, v);\n") % nonTerminal;
						break;
					}
					else if (!defval.isLeaf)
					{
						mSource << mTabs << boost::format("p = Traverse_%1%(p, v);\n") % nonTerminal;
						break;
					}
				}
				mSource << mTabs << boost::format("p = Parse_%1%(p);\n") % nonTerminal;
				break;
			}
				
			case ExpressionType_Range:
			{
				mSource << mTabs << boost::format("if (p->value >= \'%1%\' && p->value <= \'%2%\') ++p; else p = 0;\n")
					% EscapeChar(expr.GetFirst())
					% EscapeChar(expr.GetLast());
				break;
			}
				
			case ExpressionType_Char:
			{
				mSource << mTabs << boost::format("if (p->value == \'%1%\') ++p; else p = 0;\n") % EscapeChar(expr.GetChar());
				break;
			}
				
			case ExpressionType_Dot:
			{
				mSource << mTabs << "if (p->value != 0) ++p; else p = 0;\n";
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
	
	static std::string Not(std::string _expr)
	{
		return "!" + _expr;
	}
	
	void DefineBacktrack(int& _backtrackIndex, bool _visit)
	{
		if (_backtrackIndex == -1)
		{
			_backtrackIndex = mNextBacktrackIndex++;
			mSource << mTabs << boost::format("Node* %1% = p;\n") % BacktrackVar(_backtrackIndex);
			if (mTraverse && _visit)
				mSource << mTabs << boost::format("size_t %1% = v.size();\n") % BacktrackVar(_backtrackIndex, 's');
		}
	}
	
	void Backtrack(int _backtrackIndex, bool _visit)
	{
		mSource << mTabs << boost::format("p = %1%;\n") % BacktrackVar(_backtrackIndex);
		if (mTraverse && _visit)
			mSource << mTabs << boost::format("v.erase(v.begin() + %1%, v.end());\n") % BacktrackVar(_backtrackIndex, 's');
	}
	
	static std::string BacktrackVar(int _backtrackIndex, char _prefix = 'b')
	{
		assert(_backtrackIndex != -1);
		if (_backtrackIndex == 1)
			return str(boost::format("%1%") % _prefix);
		else
			return str(boost::format("%1%%2%") % _prefix % _backtrackIndex);
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
		
		if (i->second.isMemoized)
			pDef->ShowSection("isMemoized");
			
		if (i->second.isLeaf)
			pDef->ShowSection("isLeaf");
		else
			pDef->ShowSection("isInternal");
		
		std::ostringstream parseCodeStream;
		ParserGenerator parserGenerator(parseCodeStream, _grammar);
		parserGenerator.Emit(i->second);
		
		std::string parseCodeFilename = "parseCode_" + i->first;
		std::string parseCode = parseCodeStream.str();
		ctemplate::StringToTemplateCache(parseCodeFilename, parseCode, ctemplate::STRIP_BLANK_LINES);
		
		pDef->AddIncludeDictionary("parseCode")->SetFilename(parseCodeFilename);
		
		std::ostringstream traverseCodeStream;
		ParserGenerator traverserGenerator(traverseCodeStream, _grammar, true);
		traverserGenerator.Emit(i->second);
		
		std::string traverseCodeFilename = "traverseCode_" + i->first;
		std::string traverseCode = traverseCodeStream.str();
		ctemplate::StringToTemplateCache(traverseCodeFilename, traverseCode, ctemplate::STRIP_BLANK_LINES);

		pDef->AddIncludeDictionary("traverseCode")->SetFilename(traverseCodeFilename);
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
