#include "Common.h"
#include "GenerateParser.h"
#include "Grammar.h"
#include "Utility.h"

#include <ctemplate/template.h>

using namespace std;
using namespace boost;
using namespace ctemplate;

class ParserGenerator
{
	ostream& mSource;
	const Grammar& mGrammar;
	int mTabs;
	int mNextBacktrackIndex;
	int mNextCharIndex;
	bool mTraverse;
	
public:
	ParserGenerator(ostream& _source, const Grammar& _grammar, bool _traverse = false)
	: mSource(_source)
	, mGrammar(_grammar)
	, mTabs(2)
	, mNextBacktrackIndex(1)
	, mNextCharIndex(0)
	, mTraverse(_traverse)
	{
	}
	
	void Emit(const Expression& expr, int _backtrackIndex)
	{
		switch (expr.GetType())
		{
			case ExpressionType_Empty:
			{
				Line("r = true;");
				break;
			}
				
			case ExpressionType_Choice:
			{
				const Expression::Group& group = expr.GetGroup();
				bool mayUndoVisit = !group.first.isLeaf;
				DefineBacktrack(_backtrackIndex, mayUndoVisit);
				Emit(group.first, _backtrackIndex);
				If("!r");
				OpenBlock();
				Backtrack(_backtrackIndex, mayUndoVisit);
				Emit(group.second, _backtrackIndex);
				CloseBlock();
				break;
			}
				
			case ExpressionType_Sequence:
			{
				const Expression::Group& group = expr.GetGroup();
				Emit(group.first, _backtrackIndex);
				If("r");
				OpenBlock();
				Emit(group.second, -1);
				CloseBlock();
				break;
			}

			case ExpressionType_Not:
			{
				bool traverse = mTraverse;
				mTraverse = false;
				DefineBacktrack(_backtrackIndex, false);
				Emit(expr.GetChild(), _backtrackIndex);
				Line("r = !r;");
				Line(format("p = %1%;") % BacktrackVar(_backtrackIndex));
				mTraverse = traverse;
				break;
			}
				
			case ExpressionType_ZeroOrMore:
			{
				_backtrackIndex = -1;
				Line("for (;;)");
				OpenBlock();
				const Expression& child = expr.GetChild();
				bool mayUndoVisit = !child.isLeaf;
				DefineBacktrack(_backtrackIndex, mayUndoVisit);
				Emit(child, _backtrackIndex);
				If("!r");
				OpenBlock();
				Backtrack(_backtrackIndex, mayUndoVisit);
				Line("break;");
				CloseBlock();
				CloseBlock();
				Line("r = true;");
				break;
			}
				
			case ExpressionType_NonTerminal:
			{
				const string& nonTerminal = expr.GetNonTerminal();
				const Def& def = *mGrammar.defs.find(nonTerminal);
				const DefValue& defval = def.second;
				if (mTraverse)
				{
					if (defval.isNode)
						Line(format("r = Visit(_ctx, SymbolType_%1%, p, v);") % nonTerminal);
					else if (defval.isMemoized)
						Line(format("r = TraverseSkip(_ctx, SkipType_%1%, p, v);") % nonTerminal);
					else
						Line(format("r = Traverse_%1%(_ctx, p, v);") % nonTerminal);
				}
				else
				{
					if (defval.isNode)
						Line(format("r = ParseSymbol(_ctx, SymbolType_%1%, p);") % nonTerminal);
					else if (defval.isMemoized)
						Line(format("r = ParseSkip(_ctx, SkipType_%1%, p);") % nonTerminal);
					else
						Line(format("r = Parse_%1%(_ctx, p);") % nonTerminal);
				}
				break;
			}
				
			case ExpressionType_Range:
			{
				int charIndex = mNextCharIndex++;
				Line(format("Char c%1% = *p++;") % charIndex);
				Line(format("r = (c%1% >= \'%2%\' && c%1% <= \'%3%\');") % charIndex % EscapeChar(expr.GetFirst()) % EscapeChar(expr.GetLast()));
				break;
			}
				
			case ExpressionType_Char:
			{
				int charIndex = mNextCharIndex++;
				Line(format("Char c%1% = *p++;") % charIndex);
				Line(format("r = (c%1% == \'%2%\');") % charIndex % EscapeChar(expr.GetChar()));
				break;
			}
				
			case ExpressionType_Dot:
			{
				int charIndex = mNextCharIndex++;
				Line(format("Char c%1% = *p++;") % charIndex);
				Line(format("r = (c%1% != 0);") % charIndex);
				break;
			}
				
			default:
			{
				assert(false);
			}
		}
	}
	
	template <class T>
	void Line(const T& _stmt)
	{
		int tabs = mTabs;
		while (tabs--)
			mSource.put('\t');
		mSource << _stmt;
		mSource.put('\n');
	}
	
	void If(string _cond)
	{
		Line(format("if (%1%)") % _cond);
	}
	
	void OpenBlock()
	{
		Line("{");
		++mTabs;
	}
	
	void CloseBlock()
	{
		--mTabs;
		Line("}");
	}
	
	static string Not(string _expr)
	{
		return "!" + _expr;
	}
	
	void DefineBacktrack(int& _backtrackIndex, bool _mayUndoVisit)
	{
		if (_backtrackIndex == -1)
		{
			_backtrackIndex = mNextBacktrackIndex++;
			Line(format("CharItr %1% = p;") % BacktrackVar(_backtrackIndex));
			if (mTraverse && _mayUndoVisit)
				Line(format("size_t %1% = v.size();\n") % BacktrackVar(_backtrackIndex, 's'));
		}
	}
	
	void Backtrack(int _backtrackIndex, bool _undoVisit)
	{
		Line(format("p = %1%;\n") % BacktrackVar(_backtrackIndex));
		if (mTraverse && _undoVisit)
			Line(format("v.erase(v.begin() + %1%, v.end());\n") % BacktrackVar(_backtrackIndex, 's'));
	}
	
	static string BacktrackVar(int _backtrackIndex, char _prefix = 'b')
	{
		assert(_backtrackIndex != -1);
		if (_backtrackIndex == 1)
			return str(format("%1%") % _prefix);
		else
			return str(format("%1%%2%") % _prefix % _backtrackIndex);
	}
};

static void WriteAutoGenNotice(ostream& _os, const string& _srcPath)
{
	time_t now = time(NULL);
	char time_buffer[26];

	format fmt(
		"// This file was automatically generated by IPG on %1%"
		"// (from %2%)\n"
		"// DO NOT EDIT!\n");
		
	_os << fmt % ctime_r(&now, time_buffer) % _srcPath;
}

void GenerateParser(string _srcPath, string _folder, string _name, const Grammar& _grammar)
{
	TemplateDictionary dict(_name);
	dict.SetValue("namespace", _name);

	Defs::const_iterator i, iEnd = _grammar.defs.end();
	for (i = _grammar.defs.begin(); i != iEnd; ++i)
	{
		TemplateDictionary* pDef = dict.AddSectionDictionary("def");
		pDef->SetValue("name", i->first);
	
		if (i->second.isNode)
			pDef->ShowSection("isNode");
		else if (i->second.isMemoized)
			pDef->ShowSection("isSkip");
		
		ostringstream parseCodeStream;
		ParserGenerator parserGenerator(parseCodeStream, _grammar);
		parserGenerator.Emit(i->second, -1);
		
		string parseCodeFilename = "parseCode_" + i->first;
		string parseCode = parseCodeStream.str();
		StringToTemplateCache(parseCodeFilename, parseCode, STRIP_BLANK_LINES);
		
		pDef->AddIncludeDictionary("parseCode")->SetFilename(parseCodeFilename);
		
		ostringstream traverseCodeStream;
		ParserGenerator traverserGenerator(traverseCodeStream, _grammar, true);
		traverserGenerator.Emit(i->second, -1);
		
		string traverseCodeFilename = "traverseCode_" + i->first;
		string traverseCode = traverseCodeStream.str();
		StringToTemplateCache(traverseCodeFilename, traverseCode, STRIP_BLANK_LINES);

		pDef->AddIncludeDictionary("traverseCode")->SetFilename(traverseCodeFilename);
	}

	string headerText;
	if (!ExpandTemplate("Parser.h.tpl", STRIP_BLANK_LINES, &dict, &headerText))
		throw runtime_error("CTemplate Parser.h.tpl expansion failed!");
	
	ofstream headerFile;
	headerFile.exceptions(ofstream::failbit | ofstream::badbit);
	headerFile.open((_folder + _name + ".h").c_str());
	WriteAutoGenNotice(headerFile, _srcPath);
	headerFile << format("#ifndef %1%_H\n#define %1%_H\n\n") % to_upper_copy(_name);
	headerFile << headerText;
	headerFile << "\n#endif\n";
	
	dict.SetValue("header", headerText);

	string sourceText;
	if (!ExpandTemplate("Parser.cpp.tpl", STRIP_BLANK_LINES, &dict, &sourceText))
		throw runtime_error("CTemplate Parser.cpp.tpl expansion failed!");
	
	ofstream sourceFile;
	sourceFile.exceptions(ofstream::failbit | ofstream::badbit);
	string sourcePath = _folder + _name + ".cpp";
	sourceFile.open(sourcePath.c_str());
	WriteAutoGenNotice(sourceFile, _srcPath);
	sourceFile << sourceText;	
}
