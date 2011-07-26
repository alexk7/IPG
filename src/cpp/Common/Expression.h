#ifndef EXPRESSION_H_
#define EXPRESSION_H_

enum ExpressionType
{
	ExpressionType_Empty,
	
	//Group
	ExpressionType_Choice,
	ExpressionType_Sequence,
	
	//Container
	ExpressionType_Not,
	ExpressionType_ZeroOrMore,
	
	//SingleChar
	ExpressionType_Range,
	ExpressionType_Char,
	ExpressionType_Dot,
	
	//NonTerminal
	ExpressionType_NonTerminal,
	
	ExpressionType_Count
};

class Expression
{
public:
	typedef std::pair<Expression, Expression> Group;

	Expression();
	explicit Expression(const Expression& _rhs);
	~Expression();
	bool operator==(const Expression& _rhs) const;
	void Swap(Expression& _other);
	
	void AddGroupItem(ExpressionType _groupType, Expression& _item);
	void SetContainer(ExpressionType _containerType, Expression& _child);
	void SetNonTerminal(std::string _identifier);
	void SetRange(char _first, char _last);
	void SetChar(char _value);
	void SetDot();
	void SetEmpty();
	
	void Print(std::ostream& _os, ExpressionType parentType = ExpressionType_Empty) const;
	ExpressionType GetType() const;
	
	char GetChar() const;
	char GetFirst() const;
	char GetLast() const;
	
	std::string GetNonTerminal() const;
	
	Group& GetGroup();
	const Group& GetGroup() const;
	
	Expression& GetChild();
	const Expression& GetChild() const;
	
	bool isLeaf;
	
private:
	void PrintChildren(std::ostream& _os, const char* _separator, ExpressionType _parentType) const;
	void PrintChildWithPrefix(std::ostream& _os, char _prefix) const;
	void PrintChildWithSuffix(std::ostream& _os, char _prefix) const;
	static void PrintRangeChar(std::ostream& _os, char _char);
	
	union Data
	{
		char chars[2];
		char* pString;
		Expression* pExpression;
		Group* pGroup;
	};
	
	ExpressionType mType;
	Data mData;
};

bool IsGroup(ExpressionType _type);
bool IsContainer(ExpressionType _type);
bool IsSingleChar(ExpressionType _type);

void swap(Expression& _lhs, Expression& _rhs);
std::ostream& operator<<(std::ostream& _os, const Expression& _e);

#endif /* EXPRESSION_H_ */
