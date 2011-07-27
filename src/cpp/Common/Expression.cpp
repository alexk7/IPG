#include "Common.h"
#include "Expression.h"
#include <cassert>
#include <cstdlib>
#include <cstring>

using namespace std;

Expression::Expression() : mType(ExpressionType_Empty), isLeaf(false)
{
	memset(&mData, 0, sizeof(mData));
}

Expression::Expression(const Expression& _rhs) : mType(_rhs.mType), isLeaf(false)
{
	switch (mType)
	{
		case ExpressionType_Empty:
		case ExpressionType_Dot:
		case ExpressionType_Range:
		case ExpressionType_Char:
			memcpy(&mData, &_rhs.mData, sizeof(mData));
			break;
		
		case ExpressionType_NonTerminal:
			mData.pString = new char[strlen(_rhs.mData.pString)];
			strcpy(mData.pString, _rhs.mData.pString);
			break;

		case ExpressionType_Choice:
		case ExpressionType_Sequence:
			mData.pGroup = new Group(*_rhs.mData.pGroup);
			break;
		
		case ExpressionType_Not:
		case ExpressionType_ZeroOrMore:
			mData.pExpression = new Expression(*_rhs.mData.pExpression);
			break;
				
		default:
			assert(false);
	}
}

Expression::~Expression()
{
	switch (mType)
	{
		case ExpressionType_Empty:
		case ExpressionType_Dot:
		case ExpressionType_Range:
		case ExpressionType_Char:
			break;
		
		case ExpressionType_NonTerminal:
			delete [] mData.pString;
			break;

		case ExpressionType_Choice:
		case ExpressionType_Sequence:
			delete mData.pGroup;
			break;
		
		case ExpressionType_Not:
		case ExpressionType_ZeroOrMore:
			delete mData.pExpression;
			break;
				
		default:
			assert(false);
	}
}

bool Expression::operator==(const Expression& _rhs) const
{
	if (mType != _rhs.mType)
		return false;
		
	switch (mType)
	{
		case ExpressionType_Empty:
		case ExpressionType_Dot:
			return true;
		
		case ExpressionType_Range:
		case ExpressionType_Char:
			return memcmp(&mData, &_rhs.mData, sizeof(mData)) == 0;
		
		case ExpressionType_NonTerminal:
			return strcmp(mData.pString, _rhs.mData.pString) == 0;

		case ExpressionType_Choice:
		case ExpressionType_Sequence:
			return *mData.pGroup == *_rhs.mData.pGroup;
		
		case ExpressionType_Not:
		case ExpressionType_ZeroOrMore:
			return *mData.pExpression == *_rhs.mData.pExpression;
				
		default:
			assert(false);
			return false;		
	}
}

void Expression::Swap(Expression& _other)
{
	using std::swap;
	swap(mType, _other.mType);
	swap(mData, _other.mData);
}

void Expression::AddGroupItem(ExpressionType _groupType, Expression& _child)
{
	assert(IsGroup(_groupType));
	
	if (mType == ExpressionType_Empty)
	{
		Swap(_child);
	}
	else if (mType == _groupType)
	{
		GetGroup().second.AddGroupItem(_groupType, _child);
	}
	else
	{
		assert(this != &_child);
		
		Expression me;
		Swap(me);
		
		mData.pGroup = new Group;
		mType = _groupType;
		mData.pGroup->first.Swap(me);
		mData.pGroup->second.Swap(_child);
	}
}

void Expression::SetContainer(ExpressionType _containerType, Expression& _child)
{
	if (_containerType == ExpressionType_Empty)
	{
		Swap(_child);
	}
	else
	{
		assert(IsContainer(_containerType));
		
		//This is important because we may have this == &_child
		Expression child;
		child.Swap(_child);
		
		SetEmpty();
		mData.pExpression = new Expression;
		mType = _containerType;
		mData.pExpression->Swap(child);
	}
}

void Expression::SetNonTerminal(std::string _identifier)
{
	SetEmpty();
	mData.pString = new char[_identifier.size() + 1];
	strcpy(mData.pString, _identifier.c_str());
	mType = ExpressionType_NonTerminal;
}

void Expression::SetRange(char _first, char _last)
{
	SetEmpty();
	mData.chars[0] = _first;
	mData.chars[1] = _last;
	mType = ExpressionType_Range;
}

void Expression::SetChar(char _value)
{
	SetEmpty();
	mData.chars[0] = _value;
	mType = ExpressionType_Char;
}

void Expression::SetDot()
{
	SetEmpty();
	mType = ExpressionType_Dot;
}

void Expression::SetEmpty()
{
	Expression me;
	Swap(me);
}

void Expression::Print(std::ostream& _os, ExpressionType parentType) const
{
	switch (mType)
	{
		case ExpressionType_Empty:                                              break;
		case ExpressionType_Choice:      PrintChildren(_os, " / ", parentType); break;
		case ExpressionType_Sequence:    PrintChildren(_os, " ", parentType);   break;
		case ExpressionType_Not:         PrintChildWithPrefix(_os, '!');        break;
		case ExpressionType_ZeroOrMore:  PrintChildWithSuffix(_os, '*');        break;
		case ExpressionType_NonTerminal: _os << GetNonTerminal();               break;
		case ExpressionType_Dot:         _os.put('.');                          break;
			
		case ExpressionType_Char:
		{
			_os.put('\'');
			char c = GetChar();
			switch (c)
			{
				case '\\': _os << "\\\\"; break;
				case '\n': _os << "\\n";  break;
				case '\r': _os << "\\r";  break;
				case '\t': _os << "\\t";  break;
				case '\'': _os << "\\\'"; break;
				default:   _os.put(c);    break;
			}
			_os.put('\'');
			break;
		}
			
		case ExpressionType_Range:
		{
			_os.put('[');
			PrintRangeChar(_os, GetFirst());
			_os.put('-');
			PrintRangeChar(_os, GetLast());
			_os.put(']');
			break;
		}
			
		default: assert(false);
	}
}

void Expression::PrintChildren(std::ostream& _os, const char* _separator, ExpressionType parentType) const
{
	const Group& g = GetGroup();
	
	bool needParens = parentType > mType;
	if (needParens)
		_os.put('(');
	
	g.first.Print(_os, mType);
	_os << _separator;
	g.second.Print(_os, mType);
	
	if (needParens)
		_os.put(')');
}

void Expression::PrintChildWithPrefix(std::ostream& _os, char _prefix) const
{
	_os.put(_prefix);
	GetChild().Print(_os, mType);
}

void Expression::PrintChildWithSuffix(std::ostream& _os, char _suffix) const
{
	GetChild().Print(_os, mType);
	_os.put(_suffix);
}

void Expression::PrintRangeChar(std::ostream& _os, char _char)
{
	switch (_char)
	{
		case '\\': _os << "\\\\";  break;
		case '\n': _os << "\\n";   break;
		case '\r': _os << "\\r";   break;
		case '\t': _os << "\\t";   break;
		case '[':  _os << "\\[";   break;
		case ']':  _os << "\\]";   break;
		default:   _os.put(_char); break;
	}
}

char Expression::GetChar() const
{
	assert(mType == ExpressionType_Char);
	return mData.chars[0];
}

char Expression::GetFirst() const
{
	assert(mType == ExpressionType_Range);
	return mData.chars[0];
}

char Expression::GetLast() const
{
	assert(mType == ExpressionType_Range);
	return mData.chars[1];
}

string Expression::GetNonTerminal() const
{
	assert(mType == ExpressionType_NonTerminal);
	return mData.pString;
}

Expression::Group& Expression::GetGroup()
{
	assert(IsGroup(mType));
	return *mData.pGroup;
}

const Expression::Group& Expression::GetGroup() const
{
	assert(IsGroup(mType));
	return *mData.pGroup;
}

Expression& Expression::GetChild()
{
	assert(IsContainer(mType));
	return *mData.pExpression;
}

const Expression& Expression::GetChild() const
{
	assert(IsContainer(mType));
	return *mData.pExpression;
}

bool IsGroup(ExpressionType _type)
{
	switch (_type)
	{
		case ExpressionType_Choice:
		case ExpressionType_Sequence: return true;
		default: /* not a group */    return false;
	}
}

bool IsContainer(ExpressionType _type)
{
	switch (_type)
	{
		case ExpressionType_Not:
		case ExpressionType_ZeroOrMore: return true;
		default: /* not a container */  return false;
	}
}

bool IsSingleChar(ExpressionType _type)
{
	switch (_type)
	{
		case ExpressionType_Range:
		case ExpressionType_Char:
		case ExpressionType_Dot:         return true;
		default: /* not a single char */ return false;
	}
}

ExpressionType Expression::GetType() const
{
	return mType;
}

void swap(Expression& _lhs, Expression& _rhs)
{
	_lhs.Swap(_rhs);
}

std::ostream& operator<<(std::ostream& _os, const Expression& _e)
{
	_e.Print(_os);
	return _os;
}
