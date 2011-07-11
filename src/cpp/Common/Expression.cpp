#include "Common.h"
#include "Expression.h"

template <class T>
T& Expression::SetType(ExpressionType _type)
{
	mType = _type;
	mData = T();
	return boost::get<T>(mData);
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
		
		Group& group = SetType<Group>(_groupType);
		group.first.Swap(me);
		group.second.Swap(_child);
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
		Expression tmp;
		tmp.Swap(_child);
		
		Expression& child = SetType<Expression>(_containerType);
		child.Swap(tmp);
	}
}

void Expression::SetNonTerminal(const std::string& _identifier)
{
	mType = ExpressionType_NonTerminal;
	mData = _identifier;
}

void Expression::SetRange(char _first, char _last)
{
	mType = ExpressionType_Range;
	mData = std::make_pair(_first, _last);
}

void Expression::SetChar(char _value)
{
	mType = ExpressionType_Char;
	mData = _value;
}

void Expression::Print(std::ostream& _os, ExpressionType parentType) const
{
	switch (mType)
	{
		case ExpressionType_Empty:       _os << "ε";                            break;
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
			std::pair<char, char> p = boost::get<std::pair<char, char> >(mData);
			_os.put('[');
			PrintRangeChar(_os, p.first);
			_os.put('-');
			PrintRangeChar(_os, p.second);
			_os.put(']');
			break;
		}
			
		default: assert(false);
	}
}

void Expression::PrintChildren(std::ostream& _os, const char* _separator, ExpressionType parentType) const
{
	const Group& p = boost::get<Group>(mData);
	
	bool needParens = parentType > mType;
	if (needParens)
		_os.put('(');
	
	p.first.Print(_os, mType);
	_os << _separator;
	p.second.Print(_os, mType);
	
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
	return boost::get<char>(mData);
}

char Expression::GetFirst() const
{
	assert(mType == ExpressionType_Range);
	return boost::get<std::pair<char, char> >(mData).first;
}

char Expression::GetLast() const
{
	assert(mType == ExpressionType_Range);
	return boost::get<std::pair<char, char> >(mData).second;
}

const std::string& Expression::GetNonTerminal() const
{
	assert(mType == ExpressionType_NonTerminal);
	return boost::get<std::string>(mData);
}

Expression::Group& Expression::GetGroup()
{
	assert(IsGroup(mType));
	return boost::get<Group>(mData);
}

const Expression::Group& Expression::GetGroup() const
{
	assert(IsGroup(mType));
	return boost::get<Group>(mData);
}

Expression& Expression::GetChild()
{
	assert(IsContainer(mType));
	return boost::get<Expression>(mData);
}

const Expression& Expression::GetChild() const
{
	assert(IsContainer(mType));
	return boost::get<Expression>(mData);
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
