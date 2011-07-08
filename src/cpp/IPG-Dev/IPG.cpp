#include "../Common/Common.h"
#include "../Common/Grammar.h"
#include "../Common/GenerateParser.h"
#include "../Common/FlattenGrammar.h"
#include "../Common/ReadFile.h"
#include "../Common/Parser.h.tpl.h"
#include "../Common/Parser.cpp.tpl.h"

#include "./PEGParser.h"

#include <boost/shared_ptr.hpp>
#include <ctemplate/template.h>

using namespace PEGParser;

struct PTNodeChild
{
	PTNodeType type;
	PTNode* pNode;
};

typedef std::vector<PTNodeChild> PTNodeChildren;

//Instead of just gathering them all, I should use some kind of
//coroutine or generator but it's not supported in C++.
//Or some ugly thread thingy.
//Bleh.
struct PTNodeChildrenGetter : PTNodeVisitor
{
	PTNodeChildren children;
	virtual void operator()(PTNode* _symbol, PTNodeType _type)
	{
		PTNodeChild child;
		child.type = _type;
		child.pNode = _symbol;
		children.push_back(child);
	}
};

static std::string ToString(PTNode* _pNode, PTNodeType _type)
{
	std::string result;
	if (_pNode)
	{
		if (PTNode* pEnd = _pNode->end[_type])
		{
			for (PTNode* p = _pNode; p != pEnd; ++p)
				result.push_back(p->value);
		}
	}
	return result;
}

class SymbolItr
{
public:
	SymbolItr(PTNode* _pNode)
	:
	mPtr(_pNode)
	{
	}
	
	SymbolItr& operator++()
	{
		++mPtr;
		return *this;
	}
	
	operator bool() const
	{
		return !mPtr;
	}
	
	char operator*()
	{
		return mPtr->value;
	}
	
	friend SymbolItr operator+(const SymbolItr& _lhs, int _rhs)
	{
		return SymbolItr(_lhs.mPtr + _rhs);
	}
	
private:
	PTNode* mPtr;
};

class PTChildItr
{
public:
	PTChildItr(const PTChildItr& _iOther)
	: mType(_iOther.mType)
	, mpSiblings(_iOther.mpSiblings)
	, miCurrent(_iOther.miCurrent)
	{}
	
	PTChildItr(boost::shared_ptr<PTNodeChildren> _pSiblings, PTNodeType _childType)
	: mType(_childType)
	, mpSiblings(_pSiblings)
	{
		miCurrent = mpSiblings->begin();
		SkipSiblingsWithWrongType(_childType);
	}
	
	PTChildItr(const PTChildItr& _iOther, PTNodeType _childType)
	: mType(_childType)
	,	mpSiblings(_iOther.mpSiblings)
	,	miCurrent(_iOther.miCurrent)
	{
		SkipSiblingsWithWrongType(_childType);
	}
		
	operator PTNode*() const
	{
		return *this ? miCurrent->pNode : 0;
	}
	
	operator bool() const
	{
		return miCurrent != mpSiblings->end();
	}
	
	operator SymbolItr() const
	{
		PTNode* pNode = *this;
		return pNode;
	}
	
	PTChildItr& operator++()
	{
		mpChildren.reset();
		GoToNext(mType);
		return *this;
	}
	
	std::string ToString() const
	{
		PTNode* pNode = *this;
		return ::ToString(pNode, mType);
	}
	
	SymbolItr Begin() const
	{
		PTNode* pNode = *this;
		return pNode;
	}
	
	SymbolItr End() const
	{
		PTNode* pNode = *this;
		return pNode ? pNode->end[mType] : 0;
	}
	
	PTNodeType GetType() const { return mType; }
	
	PTChildItr GetChild(PTNodeType _childT)
	{
		return PTChildItr(GetChildren(), _childT);
	}

	PTChildItr GetNext(PTNodeType _childT)
	{
		return PTChildItr(*this, _childT);
	}
	
	boost::shared_ptr<PTNodeChildren> GetChildren()
	{
		if (!mpChildren)
		{
			PTNodeChildrenGetter getter;
			Traverse(mType, *this, getter);
			mpChildren.reset(new PTNodeChildren);
			mpChildren->swap(getter.children);			
		}
		
		return mpChildren;
	}
	
private:
	void GoToNext(PTNodeType _childType)
	{
		++miCurrent;
		SkipSiblingsWithWrongType(_childType);
	}
	
	void SkipSiblingsWithWrongType(PTNodeType _childType)
	{
		PTNodeChildren::iterator iEnd = mpSiblings->end();
		while (miCurrent != iEnd && miCurrent->type != _childType)
			++miCurrent;
	}
	
	PTNodeType mType;
	boost::shared_ptr<PTNodeChildren> mpSiblings;
	PTNodeChildren::iterator miCurrent;
	boost::shared_ptr<PTNodeChildren> mpChildren;
};

class PTItr
{
public:
	PTItr(PTNodeType _type, PTNode* _pNode = 0) : mType(_type), mPtr(Check(_type, _pNode)) {}
	PTItr(const PTChildItr& _iOther) : mType(_iOther.GetType()), mPtr(_iOther) {}
		
	operator bool() const
	{
		return mPtr != 0;
	}
	
	operator PTNode*() const
	{
		return mPtr;
	}
	
	std::string ToString() const
	{
		return ::ToString(mPtr, mType);
	}
	
	PTNodeType GetType() const { return mType; }
	
	PTChildItr GetChild(PTNodeType _childT)
	{
		return PTChildItr(GetChildren(), _childT);
	}
	
	boost::shared_ptr<PTNodeChildren> GetChildren()
	{
		if (!mpChildren)
		{
			PTNodeChildrenGetter getter;
			Traverse(mType, *this, getter);
			mpChildren.reset(new PTNodeChildren);
			mpChildren->swap(getter.children);			
		}
		
		return mpChildren;
	}
	
	
private:
	static PTNode* Check(PTNodeType _type, PTNode* _pNode)
	{
		return (_pNode && _pNode->end[_type]) ? _pNode : 0;
	}
	
	PTNodeType mType;
	PTNode* mPtr;
	boost::shared_ptr<PTNodeChildren> mpChildren;
};

static bool ReadFile(std::vector<PTNode>& _symbols, const char* _filename)
{
	_symbols.clear();
	
	std::vector<char> text;
	if (ReadFile(text, _filename))
	{
		std::size_t size = text.size();
		_symbols.resize(size+1);
		
		for (size_t i = 0; i < size; ++i)
		{
			_symbols[i].value = text[i];
		}
		
		_symbols[size].value = 0;
		
		return true;
	}
	
	return false;
}

static char GetChar(SymbolItr _iChar)
{
	char c = *_iChar;
	if (c == '\\')
	{
		c = *++_iChar;
		if (c == 'n')
		{
			c = '\n';
		}
		else if (c == 'r')
		{
			c = '\r';
		}
		else if (c == 't')
		{
			c = '\t';
		}
		else if (c >= '1' && c <= '9')
		{
			c -= '0';
			for (;;)
			{
				char digit = *++_iChar;
				if (digit < '0' || digit > '9')
					break;
				c = (10 * c) + (digit - '0');
			}
		}
	}
	return c;
}

static void ConvertExpression(Expression& _expr, PTItr _iExpr)
{
	Expression sequence, primary, charExpr;
	
	for (PTChildItr iSeq = _iExpr.GetChild(PTNodeType_Sequence); iSeq; ++iSeq)
	{
		for (PTChildItr iPrefix = iSeq.GetChild(PTNodeType_Prefix); iPrefix; ++iPrefix)
		{
			char cPrefix = *iPrefix.Begin();
			PTChildItr iSuffix = iPrefix.GetChild(PTNodeType_Suffix);
			PTChildItr iPrimary = iSuffix.GetChild(PTNodeType_Primary);
			
			if (PTChildItr iId = iPrimary.GetChild(PTNodeType_Identifier))
			{
				primary.SetNonTerminal(iId.ToString());
			}
			else if (PTChildItr iExpr = iPrimary.GetChild(PTNodeType_Expression))
			{
				ConvertExpression(primary, iExpr);
			}
			else if (PTChildItr iLiteral = iPrimary.GetChild(PTNodeType_Literal))
			{
				for (PTChildItr iChar = iLiteral.GetChild(PTNodeType_Char); iChar; ++iChar)
				{
					charExpr.SetChar(GetChar(iChar));
					primary.AddGroupItem(ExpressionType_Sequence, charExpr);
				}
			}
			else if (PTChildItr iClass = iPrimary.GetChild(PTNodeType_Class))
			{
				for (PTChildItr iRange = iClass.GetChild(PTNodeType_Range); iRange; ++iRange)
				{
					PTChildItr iChar = iRange.GetChild(PTNodeType_Char);
					char firstChar = GetChar(iChar);
					if (++iChar)
					{
						char lastChar = GetChar(iChar);
						charExpr.SetRange(firstChar, lastChar);
					}
					else
					{
						charExpr.SetChar(firstChar);
					}
					primary.AddGroupItem(ExpressionType_Choice, charExpr);
				}
			}
			else
			{
				primary.SetDot();
			}
			
			char cSuffix = *iPrimary.End();
			if (cSuffix == '?')
			{
				Expression empty;
				primary.AddGroupItem(ExpressionType_Choice, empty);
			}
			else if (cSuffix == '*')
			{
				primary.SetContainer(ExpressionType_ZeroOrMore, primary);
			}
			else if (cSuffix == '+')
			{
				Expression zeroOrMore = primary;
				zeroOrMore.SetContainer(ExpressionType_ZeroOrMore, zeroOrMore);
				primary.AddGroupItem(ExpressionType_Sequence, zeroOrMore);
			}
			
			if (cPrefix == '&')
			{
				primary.SetContainer(ExpressionType_Not, primary);
				primary.SetContainer(ExpressionType_Not, primary);
			}
			else if (cPrefix == '!')
			{
				primary.SetContainer(ExpressionType_Not, primary);
			}
			
			sequence.AddGroupItem(ExpressionType_Sequence, primary);
		}
		
		_expr.AddGroupItem(ExpressionType_Choice, sequence);
	}
}

static void ConvertGrammar(Grammar& _grammar, PTItr _iGrammar)
{
	for (PTChildItr iDef = _iGrammar.GetChild(PTNodeType_Definition); iDef; ++iDef)
	{
		PTChildItr iId = iDef.GetChild(PTNodeType_Identifier);
		PTChildItr iArrow = iId.GetNext(PTNodeType_LEFTARROW);
		PTChildItr iExpr = iArrow.GetNext(PTNodeType_Expression);
		
		Expression expr;
		ConvertExpression(expr, iExpr);
		
		Def& newDef = AddDef(_grammar.defs, iId.ToString(), expr);
		char arrowType = *(iArrow.Begin() + 1);
		if (arrowType == '=')
			newDef.second.isNode = true;
		if (arrowType == '=' || arrowType == '<')
			newDef.second.isMemoized = true;
	}
}

static void RegisterTemplate(const ctemplate::TemplateString&  _key, const ctemplate::TemplateString&  _content)
{
	ctemplate::StringToTemplateCache(_key, _content, ctemplate::STRIP_BLANK_LINES);
}

static void RegisterCPlusPlusTemplates()
{
	RegisterTemplate("Parser.h.tpl", Parser_h_tpl);
	RegisterTemplate("Parser.cpp.tpl", Parser_cpp_tpl);
}

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		std::cerr << "Usage: ipg peg.txt folder name" << std::endl;
		return 1;
	}
	
	try
	{
		std::vector<PTNode> nodes;
		if (ReadFile(nodes, argv[1]))
		{
			RegisterCPlusPlusTemplates();

			std::string folder = argv[2];
			std::string name = argv[3];
			
			bool bParsed = (PEGParser::Parse(PTNodeType_Grammar, &nodes[0]) == &nodes.back());
			if (bParsed)
			{
				Grammar grammar;
				ConvertGrammar(grammar, PTItr(PTNodeType_Grammar, &nodes[0]));
				FlattenGrammar(grammar);
				
				GenerateParser(folder, name, grammar);
			}
			else
			{
				std::cout << argv[1] << ": Parsing Failed.\n";
				return 1;
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
		return 1;
	}
	
	return 0;
}
