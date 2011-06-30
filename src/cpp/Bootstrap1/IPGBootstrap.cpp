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

template <PTNodeType type>
class PTItr
{
public:
	PTItr(PTNode* _pNode = 0) : mPtr(Check(_pNode))
	{
	}
	
	PTItr& operator=(PTNode* _pNode)
	{
		mPtr = Check(_pNode);
		return *this;
	}
	
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
		return ::ToString(mPtr, type);
	}
	
private:
	PTNode* Check(PTNode* _pNode)
	{
		return (_pNode && _pNode->end[type]) ? _pNode : 0;
	}
	
	PTNode* mPtr;
};

class PTChildItrBase
{
public:
	operator PTNode*() const
	{
		return *this ? miCurrent->pNode : 0;
	}
	
	operator bool() const
	{
		return miCurrent != mpChildren->end();
	}
	
protected:
	PTChildItrBase(PTNode* _pParentNode, PTNodeType _parentType,
								 PTNodeType _childType)
	:
	mpChildren(new PTNodeChildren)
	{
		PTNodeChildrenGetter getter;
		Traverse(_parentType, _pParentNode, getter);
		mpChildren->swap(getter.children);
		
		miCurrent = mpChildren->begin();
		SkipChildrenWithWrongType(_childType);
	}
	
	PTChildItrBase(const PTChildItrBase& _iOther)
	:
	mpChildren(_iOther.mpChildren),
	miCurrent(_iOther.miCurrent)
	{
	}
	
	PTChildItrBase(const PTChildItrBase& _iOther, PTNodeType _childType)
	:
	mpChildren(_iOther.mpChildren),
	miCurrent(_iOther.miCurrent)
	{
		SkipChildrenWithWrongType(_childType);
	}
	
	void GoToNext(PTNodeType _childType)
	{
		++miCurrent;
		SkipChildrenWithWrongType(_childType);
	}
	
private:
	void SkipChildrenWithWrongType(PTNodeType _childType)
	{
		PTNodeChildren::iterator iEnd = mpChildren->end();
		while (miCurrent != iEnd && miCurrent->type != _childType)
			++miCurrent;
	}
	
	boost::shared_ptr<PTNodeChildren> mpChildren;
	PTNodeChildren::iterator miCurrent;
};

template <PTNodeType parentType, PTNodeType childType>
class PTChildItr : public PTChildItrBase
{
public:
	PTChildItr(const PTItr<parentType>& _iParent)
	:
	PTChildItrBase(_iParent, parentType, childType)
	{
	}
	
	template <PTNodeType otherParentType>
	PTChildItr(const PTChildItr<otherParentType, parentType>& _iParent)
	:
	PTChildItrBase(_iParent, parentType, childType)
	{
	}
	
	PTChildItr(const PTChildItr& _iOther)
	:
	PTChildItrBase(_iOther)
	{
	}
	
	template <PTNodeType otherChildType>
	PTChildItr(const PTChildItr<parentType, otherChildType>& _iOther)
	:
	PTChildItrBase(_iOther, childType)
	{
	}
	
	operator PTItr<childType>() const
	{
		PTNode* pNode = *this;
		return pNode;
	}
	
	operator SymbolItr() const
	{
		PTNode* pNode = *this;
		return pNode;
	}
	
	PTChildItr& operator++()
	{
		GoToNext(childType);
		return *this;
	}
	
	std::string ToString() const
	{
		PTNode* pNode = *this;
		return ::ToString(pNode, childType);
	}
	
	SymbolItr Begin() const
	{
		PTNode* pNode = *this;
		return pNode;
	}
	
	SymbolItr End() const
	{
		PTNode* pNode = *this;
		return pNode ? pNode->end[childType] : 0;
	}    
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

typedef PTItr<PTNodeType_Expression> PTItr_Expression;
typedef PTChildItr<PTNodeType_Expression, PTNodeType_Sequence>
PTItr_Expression_Sequence;
typedef PTChildItr<PTNodeType_Sequence, PTNodeType_Prefix>
PTItr_Sequence_Prefix;
typedef PTChildItr<PTNodeType_Prefix, PTNodeType_Suffix> PTItr_Prefix_Suffix;
typedef PTChildItr<PTNodeType_Suffix, PTNodeType_Primary> PTItr_Suffix_Primary;
typedef PTChildItr<PTNodeType_Primary, PTNodeType_Identifier>
PTItr_Primary_Identifier;
typedef PTChildItr<PTNodeType_Primary, PTNodeType_Expression>
PTItr_Primary_Expression;
typedef PTChildItr<PTNodeType_Primary, PTNodeType_Literal>
PTItr_Primary_Literal;
typedef PTChildItr<PTNodeType_Primary, PTNodeType_Class> PTItr_Primary_Class;
typedef PTChildItr<PTNodeType_Literal, PTNodeType_Char> PTItr_Literal_Char;
typedef PTChildItr<PTNodeType_Class, PTNodeType_Range> PTItr_Class_Range;
typedef PTChildItr<PTNodeType_Range, PTNodeType_Char> PTItr_Range_Char;

static void ConvertExpression(Expression& _expr, PTItr_Expression _iExpr)
{
	Expression sequence, primary, charExpr;
	
	for (PTItr_Expression_Sequence iSeq(_iExpr); iSeq; ++iSeq)
	{
		for (PTItr_Sequence_Prefix iPrefix(iSeq); iPrefix; ++iPrefix)
		{
			char cPrefix = *iPrefix.Begin();
			PTItr_Prefix_Suffix iSuffix(iPrefix);
			PTItr_Suffix_Primary iPrimary(iSuffix);
			
			if (PTItr_Primary_Identifier iId = iPrimary)
			{
				primary.SetNonTerminal(iId.ToString());
			}
			else if (PTItr_Primary_Expression iExpr = iPrimary)
			{
				ConvertExpression(primary, iExpr);
			}
			else if (PTItr_Primary_Literal iLiteral = iPrimary)
			{
				for (PTItr_Literal_Char iChar = iLiteral; iChar; ++iChar)
				{
					charExpr.SetChar(GetChar(iChar));
					primary.AddGroupItem(ExpressionType_Sequence, charExpr);
				}
			}
			else if (PTItr_Primary_Class iClass = iPrimary)
			{
				for (PTItr_Class_Range iRange = iClass; iRange; ++iRange)
				{
					PTItr_Range_Char iChar(iRange);
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

typedef PTItr<PTNodeType_Grammar> PTItr_Grammar;
typedef PTChildItr<PTNodeType_Grammar, PTNodeType_Definition>
PTItr_Grammar_Definition;
typedef PTChildItr<PTNodeType_Definition, PTNodeType_Identifier>
PTItr_Definition_Identifier;
typedef PTChildItr<PTNodeType_Definition, PTNodeType_LEFTARROW>
PTItr_Definition_LEFTARROW;
typedef PTChildItr<PTNodeType_Definition, PTNodeType_Expression>
PTItr_Definition_Expression;

static void ConvertGrammar(Grammar& _grammar, PTItr_Grammar _iGrammar)
{
	for (PTItr_Grammar_Definition iDef(_iGrammar); iDef; ++iDef)
	{
		PTItr_Definition_Identifier iId(iDef);
		PTItr_Definition_LEFTARROW iArrow(iId);
		PTItr_Definition_Expression iExpr(iArrow);
		
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
//	std::cout << argc << std::endl;
//	std::cout << "input file name: " << argv[1] << std::endl;
//	std::cout << "output folder name: " << argv[2] << std::endl;
//	std::cout << "output file name: " << argv[3] << std::endl;

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
			bool bParsed =
			(Parse(PTNodeType_Grammar, &nodes[0]) == &nodes.back());
			//std::cout << "Parsing " << (bParsed ? "succeeded" : "failed")
			//          << ".\n";
			
			if (bParsed)
			{
				Grammar grammar;
				ConvertGrammar(grammar, &nodes[0]);
				//std::cout << grammar;
				FlattenGrammar(grammar);
				//std::cout << "************************\n";
				//std::cout << grammar;
				
				GenerateParserSource(argv[2], argv[3], grammar);
				GenerateParserHeader(argv[2], argv[3], grammar);
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
	}
	
	return 0;
}
