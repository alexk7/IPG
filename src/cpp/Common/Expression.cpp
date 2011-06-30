#include "Common.h"
#include "Expression.h"

void Expression::Swap(Expression& _other)
{
	std::swap(mType, _other.mType);
	mChildren.swap(_other.mChildren);
	mText.swap(_other.mText);
	std::swap(mChar1, _other.mChar1);
	std::swap(mChar2, _other.mChar2);
}

void Expression::AddGroupItem(ExpressionType _groupType, Expression& _child)
{
	assert(IsGroup(_groupType));
	
	if (mType == ExpressionType_Empty)
	{
		Swap(_child);
	}
	else if (_groupType == _child.GetType())
	{
		Expressions& children = _child.GetChildren();
		Expressions::iterator i, iEnd = children.end();
		for (i = children.begin(); i != iEnd; ++i)
			AddGroupItem(_groupType, *i);
		_child.SetEmpty();
	}
	else if (mType == _groupType)
	{
		mChildren.push_back(Expression());
		mChildren.back().Swap(_child);
	}
	else
	{
		Expressions children(2);
		Swap(children.front());
		_child.Swap(children.back());
		
		mType = _groupType;
		mChildren.swap(children);
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

		Expressions children(1);
		children.back().Swap(_child);

		mType = _containerType;
		mChildren.swap(children);
	}
}

void Expression::SetNonTerminal(const std::string& _identifier)
{
	mType = ExpressionType_NonTerminal;
	mText = _identifier;
}

void Expression::SetRange(char _first, char _last)
{
	mType = ExpressionType_Range;
	mChar1 = _first;
	mChar2 = _last;
}

void Expression::SetChar(char _value)
{
	mType = ExpressionType_Char;
	mChar1 = _value;
}

void Expression::Print(std::ostream& _os, ExpressionType parentType) const
{
	switch (mType)
	{
		case ExpressionType_Choice:      PrintChildren(_os, " / ", parentType); break;
		case ExpressionType_Sequence:    PrintChildren(_os, " ", parentType); break;
		case ExpressionType_And:         PrintChildWithPrefix(_os, '&');      break;
		case ExpressionType_Not:         PrintChildWithPrefix(_os, '!');      break;
		case ExpressionType_Optional:    PrintChildWithSuffix(_os, '?');      break;
		case ExpressionType_ZeroOrMore:  PrintChildWithSuffix(_os, '*');      break;
		case ExpressionType_NonTerminal: _os << mText;                        break;
		case ExpressionType_Dot:         _os.put('.');                        break;

		case ExpressionType_Char:
		{
			_os.put('\'');            
			switch (mChar1)
			{
				case '\\': _os << "\\\\";  break;
				case '\n': _os << "\\n";  break;
				case '\r': _os << "\\r";  break;
				case '\t': _os << "\\t";  break;
				case '\'': _os << "\\\'"; break;
				default:   _os.put(mChar1);break;
			}
			_os.put('\'');
			break;
		}

		case ExpressionType_Range:
		{
			_os.put('[');
			PrintRangeChar(_os, mChar1);
			_os.put('-');
			PrintRangeChar(_os, mChar2);
			_os.put(']');
			break;
		}
            
        default: assert(false);
	}
}

void Expression::PrintChildren(std::ostream& _os, const char* _separator, ExpressionType parentType) const
{
	assert(!mChildren.empty());
	Expressions::const_iterator i = mChildren.begin(), iEnd = mChildren.end();

	bool needParens = parentType > mType && ++i != iEnd;
	if (needParens)
		_os.put('(');

	for (i = mChildren.begin();;)
	{
		i->Print(_os, mType);
		if (++i == iEnd)
			break;
		_os << _separator;
	}

	if (needParens)
		_os.put(')');
}

void Expression::PrintChildWithPrefix(std::ostream& _os, char _prefix) const
{
	_os.put(_prefix);

	assert(!mChildren.empty());
	mChildren.front().Print(_os, mType);
}

void Expression::PrintChildWithSuffix(std::ostream& _os, char _suffix) const
{
	assert(!mChildren.empty());
	mChildren.front().Print(_os, mType);

	_os.put(_suffix);
}

void Expression::PrintRangeChar(std::ostream& _os, char _char)
{
	switch (_char)
	{
		case '\\': _os << "\\\\";  break;
		case '\n': _os << "\\n";  break;
		case '\r': _os << "\\r";  break;
		case '\t': _os << "\\t";  break;
		case '[':  _os << "\\[";  break;
		case ']':  _os << "\\]";  break;
		default:   _os.put(_char);break;
	}
}

char Expression::GetChar() const
{
	assert(mType == ExpressionType_Char);
	return mChar1;
}

char Expression::GetFirst() const
{
	assert(mType == ExpressionType_Range);
	return mChar1;
}

char Expression::GetLast() const
{
	assert(mType == ExpressionType_Range);
	return mChar2;
}

const std::string& Expression::GetNonTerminal() const
{
	assert(mType == ExpressionType_NonTerminal);
	return mText;
}

Expressions& Expression::GetChildren()
{
	assert(IsGroup(mType));
	return mChildren;
}

const Expressions& Expression::GetChildren() const
{
	assert(IsGroup(mType));
	return mChildren;
}

Expression& Expression::GetChild()
{
	assert(IsContainer(mType));
	return mChildren.front();
}

const Expression& Expression::GetChild() const
{
	assert(IsContainer(mType));
	return mChildren.front();
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
		case ExpressionType_And:
		case ExpressionType_Not:
		case ExpressionType_Optional:
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
