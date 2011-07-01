#include "Common.h"
#include "Grammar.h"
#include "GenerateParser.h"
#include "FlattenGrammar.h"
#include "ReadFile.h"
#include "PEGParser.h"
#include <boost/shared_ptr.hpp>

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

class PTItr
{
public:
	PTItr(PTNodeType _type, PTNode* _pNode = 0) : mType(_type), mPtr(Check(_type, _pNode)) {}

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
	
private:
	static PTNode* Check(PTNodeType _type, PTNode* _pNode)
	{
		return (_pNode && _pNode->end[_type]) ? _pNode : 0;
	}
	
	PTNodeType mType;
	PTNode* mPtr;
};

class PTChildItr
{
public:
	friend PTChildItr GetChild(PTNodeType _childT, const PTItr& _iParent);
	friend PTChildItr GetChild(PTNodeType _childT, const PTChildItr& _iParent);
	friend PTChildItr GetNext(PTNodeType _childT, const PTChildItr& _iOther);

	PTChildItr(const PTChildItr& _iOther)
	: mType(_iOther.mType)
	, mpChildren(_iOther.mpChildren)
	, miCurrent(_iOther.miCurrent)
	{}
	
	operator PTNode*() const
	{
		return *this ? miCurrent->pNode : 0;
	}
	
	operator bool() const
	{
		return miCurrent != mpChildren->end();
	}
	
	operator PTItr() const
	{
		PTNode* pNode = *this;
		return PTItr(mType, pNode);
	}
	
	operator SymbolItr() const
	{
		PTNode* pNode = *this;
		return pNode;
	}
	
	PTChildItr& operator++()
	{
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
	  
private:
	PTChildItr(PTNode* _pParentNode, PTNodeType _parentType, PTNodeType _childType)
	: mType(_childType)
	, mpChildren(new PTNodeChildren)
	{
		PTNodeChildrenGetter getter;
		Traverse(_parentType, _pParentNode, getter);
		mpChildren->swap(getter.children);
		
		miCurrent = mpChildren->begin();
		SkipChildrenWithWrongType(_childType);
	}
	
	PTChildItr(const PTChildItr& _iOther, PTNodeType _childType)
	: mType(_childType)
	,	mpChildren(_iOther.mpChildren)
	,	miCurrent(_iOther.miCurrent)
	{
		SkipChildrenWithWrongType(_childType);
	}
	
	void GoToNext(PTNodeType _childType)
	{
		++miCurrent;
		SkipChildrenWithWrongType(_childType);
	}
	
	void SkipChildrenWithWrongType(PTNodeType _childType)
	{
		PTNodeChildren::iterator iEnd = mpChildren->end();
		while (miCurrent != iEnd && miCurrent->type != _childType)
			++miCurrent;
	}
		
	PTNodeType mType;
	boost::shared_ptr<PTNodeChildren> mpChildren;
	PTNodeChildren::iterator miCurrent;
};

inline PTChildItr GetChild(PTNodeType _childT, const PTItr& _iParent)
{
	return PTChildItr(_iParent, _iParent.GetType(), _childT);
}

inline PTChildItr GetChild(PTNodeType _childT, const PTChildItr& _iParent)
{
	return PTChildItr(_iParent, _iParent.GetType(), _childT);
}

inline PTChildItr GetNext(PTNodeType _childT, const PTChildItr& _iOther)
{
	return PTChildItr(_iOther, _childT);
}

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

//SymbolItr is like Symbol* except that * returns pSymbol->value

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
	
	for (auto iSeq = GetChild(PTNodeType_Sequence, _iExpr); iSeq; ++iSeq)
	{
		for (auto iPrefix = GetChild(PTNodeType_Prefix, iSeq); iPrefix; ++iPrefix)
		{
			char cPrefix = *iPrefix.Begin();
			auto iSuffix = GetChild(PTNodeType_Suffix, iPrefix);
			auto iPrimary = GetChild(PTNodeType_Primary, iSuffix);
			
			if (auto iId = GetChild(PTNodeType_Identifier, iPrimary))
			{
				primary.SetNonTerminal(iId.ToString());
			}
			else if (auto iExpr = GetChild(PTNodeType_Expression, iPrimary))
			{
				ConvertExpression(primary, iExpr);
			}
			else if (auto iLiteral = GetChild(PTNodeType_Literal, iPrimary))
			{
				for (auto iChar = GetChild(PTNodeType_Char, iLiteral); iChar; ++iChar)
				{
					charExpr.SetChar(GetChar(iChar));
					primary.AddGroupItem(ExpressionType_Sequence, charExpr);
				}
			}
			else if (auto iClass = GetChild(PTNodeType_Class, iPrimary))
			{
				for (auto iRange = GetChild(PTNodeType_Range, iClass); iRange; ++iRange)
				{
					auto iChar = GetChild(PTNodeType_Char, iRange);
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
				primary.SetContainer(ExpressionType_Optional, primary);
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
				primary.SetContainer(ExpressionType_And, primary);
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
	for (auto iDef = GetChild(PTNodeType_Definition, _iGrammar); iDef; ++iDef)
	{
		auto iId = GetChild(PTNodeType_Identifier, iDef);
		auto iArrow = GetNext(PTNodeType_LEFTARROW, iId);
		auto iExpr = GetNext(PTNodeType_Expression, iArrow);
		
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
			bool bParsed = (Parse(PTNodeType_Grammar, &nodes[0]) == &nodes.back());
			std::string folder = argv[2];
			std::string name = argv[3];
			
			if (bParsed)
			{
				Grammar grammar;
				ConvertGrammar(grammar, PTItr(PTNodeType_Grammar, &nodes[0]));
				FlattenGrammar(grammar);
				
				GenerateParserSource(argv[0], folder, name, grammar);
				GenerateParserHeader(folder, name, grammar);
			}
			else
			{
				std::cout << argv[1] << ": Parsing Failed.\n";
				remove((folder + name + ".cpp").c_str());
			  remove((folder + name + ".h").c_str());
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
