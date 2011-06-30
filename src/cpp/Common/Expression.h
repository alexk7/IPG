#ifndef EXPRESSION_H_
#define EXPRESSION_H_

enum ExpressionType
{
	ExpressionType_Empty,

	//Group
	ExpressionType_Choice,
	ExpressionType_Sequence,

	//Container
	ExpressionType_And,
	ExpressionType_Not,
	ExpressionType_Optional,
	ExpressionType_ZeroOrMore,

	//SingleChar
	ExpressionType_Range,
	ExpressionType_Char,
	ExpressionType_Dot,

    //NonTerminal
	ExpressionType_NonTerminal,
    
    ExpressionType_Count
};

class Expression;
typedef std::vector<Expression> Expressions;

class Expression
{
public:
	Expression() : mType(ExpressionType_Empty) {}
	void Swap(Expression& _other);

	void AddGroupItem(ExpressionType _groupType, Expression& _item);
	void SetContainer(ExpressionType _containerType, Expression& _child);
	void SetNonTerminal(const std::string& _identifier);
	void SetRange(char _first, char _last);
	void SetChar(char _value);
	void SetDot() { mType = ExpressionType_Dot; }
	void SetEmpty() { mType = ExpressionType_Empty; }
	
	void Print(std::ostream& _os,
               ExpressionType parentType = ExpressionType_Empty) const;
	ExpressionType GetType() const { return mType; }
	
	char GetChar() const;
	char GetFirst() const;
	char GetLast() const;
	
	const std::string& GetNonTerminal() const;
	
	Expressions& GetChildren();
	const Expressions& GetChildren() const;

	Expression& GetChild();
	const Expression& GetChild() const;
    
private:    
	void PrintChildren(std::ostream& _os, const char* _separator,
                       ExpressionType _parentType) const;
	void PrintChildWithPrefix(std::ostream& _os, char _prefix) const;
	void PrintChildWithSuffix(std::ostream& _os, char _prefix) const;
	static void PrintRangeChar(std::ostream& _os, char _char);

	ExpressionType mType;
	Expressions mChildren;
	std::string mText;
	char mChar1, mChar2;
};

bool IsGroup(ExpressionType _type);
bool IsContainer(ExpressionType _type);
bool IsSingleChar(ExpressionType _type);

namespace std
{
	template <>
    inline void swap(Expression& _e1, Expression& _e2) { _e1.Swap(_e2); }
}

inline std::ostream& operator<<(std::ostream& _os, const Expression& _e)
{
    _e.Print(_os);
    return _os;
}

#endif /* EXPRESSION_H_ */
