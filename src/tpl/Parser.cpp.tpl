#ifndef BOOTSTRAP_TEST
#include <cassert>
#include <stdexcept>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <boost/format.hpp>
#endif{{BI_NEWLINE}}

{{header}}

namespace
{
	struct EscapeChar
	{
		EscapeChar(char _c);
		char c;
	};{{BI_NEWLINE}}
	
	EscapeChar::EscapeChar(char _c) : c(_c)
	{
	}{{BI_NEWLINE}}
	
	std::ostream& operator<<(std::ostream& _os, EscapeChar _e)
	{
		char c = _e.c;
		switch (c)
		{
			case '\\': c = '\\'; break;
			case '\n': c = 'n';  break;
			case '\r': c = 'r';  break;
			case '\t': c = 't';  break;
			case '\'': c = '\''; break;
			case '\"': c = '\"'; break;
			
			default:
				_os.put(c);
				return _os;
		}
		
		_os.put('\\');
		_os.put(c);
		return _os;
	}
}{{BI_NEWLINE}}

const char* {{namespace}}::SymbolName({{namespace}}::SymbolType _type)
{
	switch (_type)
	{
		{{#def}}
		case SymbolType_{{name}}: return "{{name}}";
		{{/def}}
		default: throw std::runtime_error(str(boost::format("Invalid Symbol Type: %1%") % _type));
	}
}{{BI_NEWLINE}}

int {{namespace}}::Parser::SetEnd(SymbolType _type, const char* _pBegin, const char* _pEnd, int _complexity)
{
	if (_complexity > 32)
	{
		memoMap[_type][_pBegin] = _pEnd;
		return 0;
	}
	else
	{
		return _complexity;
	}
}{{BI_NEWLINE}}

const char* {{namespace}}::Parser::Parse(SymbolType _type, const char* _pBegin, int& _complexity)
{
	int complexity = 0;
	const char* p = _pBegin;
	switch (_type)
	{
		{{#def}}
		case SymbolType_{{name}}:
		{
			MemoMap::iterator i = memoMap[SymbolType_{{name}}].find(_pBegin);
			if (i != memoMap[_type].end())
				return i->second;
			{{>parseCode}}
			_complexity += SetEnd(SymbolType_{{name}}, _pBegin, p, complexity);
		  return p;
		}{{BI_NEWLINE}}
		{{/def}}
		
		default:
			assert(false);
			return 0;
	}
}{{BI_NEWLINE}}

const char* {{namespace}}::Parser::Traverse({{namespace}}::SymbolType _type, const char* _pBegin, {{namespace}}::Symbols& v, int& _complexity)
{
	int complexity = 0;
	const char* p = _pBegin;
	switch (_type)
	{
		{{#def}}
		{{#isInternal}}
		case SymbolType_{{name}}:
		{
			MemoMap::iterator i = memoMap[SymbolType_{{name}}].find(_pBegin);
			if (i != memoMap[_type].end() && i->second == 0)
				return 0;
			{{>traverseCode}}
			_complexity += SetEnd(SymbolType_{{name}}, _pBegin, p, complexity);
			return p;
		}{{BI_NEWLINE}}
		{{/isInternal}}
		{{/def}}
		{{#def}}
		{{#isLeaf}}
		case SymbolType_{{name}}:
		{{/isLeaf}}
		{{/def}}
			return Parse(_type, p, _complexity);{{BI_NEWLINE}}
			
		default:
			assert(false);
			return 0;
	}
}{{BI_NEWLINE}}

void {{namespace}}::Parser::Print(std::ostream& _os, {{namespace}}::SymbolType _type, const char* _pNode, int _tabs, int _maxLineSize)
{
	Symbols children;
	int complexity = 0;
	const char* pEnd = Traverse(_type, _pNode, children, complexity);
	if (!pEnd)
		throw std::runtime_error(str(boost::format("Parsing Failed for \"%1%\"") % SymbolName(_type)));{{BI_NEWLINE}}

	int tabCount = _tabs;
	while (tabCount--)
		_os << "    ";{{BI_NEWLINE}}
	
	_os << SymbolName(_type) << ": \"";{{BI_NEWLINE}}

	size_t lineSize = 0;
	for (const char* p = _pNode; p != pEnd; ++p)
	{
		_os << EscapeChar(*p);
		if (++lineSize >= _maxLineSize)
		{
			_os << "...";
			break;
		}
	}{{BI_NEWLINE}}
	
	_os << "\"\n";{{BI_NEWLINE}}
	
	for (Symbols::iterator i = children.begin(), iEnd = children.end(); i != iEnd; ++i)
		Print(_os, i->first, i->second, _tabs + 1, _maxLineSize);

//*
	if (_tabs == 0)
	{
		for (size_t k = 0; k < SymbolTypeCount; ++k)
		{
			_os << boost::format("Memo Count: %|20t|%1% %|40t|%2%\n") % SymbolName(SymbolType(k)) % memoMap[k].size();
		}
	}
//*/
}{{BI_NEWLINE}}

const char* {{namespace}}::Parser::Visit({{namespace}}::SymbolType _type, const char* _p, {{namespace}}::Symbols& _v)
{
	int complexity = 0;
	const char* pEnd = Parse(_type, _p, complexity);
	if (pEnd)
		_v.push_back(Symbol(_type, _p));
	return pEnd;
}{{BI_NEWLINE}}

{{namespace}}::Iterator::Iterator({{namespace}}::SymbolType _type, const char* _pNode)
:	mType(_type)
,	mpNode(_pNode)
, mpSiblings(new Symbols)
, mpParser(new Parser)
{
	if (mpNode && End())
		mpSiblings->push_back(Symbol(mType, mpNode));{{BI_NEWLINE}}
	miCurrent = mpSiblings->begin();				
}{{BI_NEWLINE}}

{{namespace}}::Iterator::Iterator(const {{namespace}}::Iterator& _iOther)
: mType(_iOther.mType)
,	mpNode(_iOther.mpNode)
, mpSiblings(_iOther.mpSiblings)
, miCurrent(_iOther.miCurrent)
,	mpParser(_iOther.mpParser)
{
}{{BI_NEWLINE}}
	
{{namespace}}::Iterator::operator bool() const
{
	return mpNode != 0;
}{{BI_NEWLINE}}

{{namespace}}::Iterator& {{namespace}}::Iterator::operator++()
{
	mpChildren.reset();
	GoToNext(mType);
	return *this;
}{{BI_NEWLINE}}
	
{{namespace}}::SymbolType {{namespace}}::Iterator::GetType() const
{
	return mType;
}{{BI_NEWLINE}}

const char* {{namespace}}::Iterator::Begin() const
{
	assert(mpNode);
	return mpNode;
}{{BI_NEWLINE}}

const char* {{namespace}}::Iterator::End() const
{
	assert(mpNode);
	int complexity = 0;
	return mpParser->Parse(mType, mpNode, complexity);
}{{BI_NEWLINE}}

{{namespace}}::Iterator {{namespace}}::Iterator::GetChild({{namespace}}::SymbolType _childT)
{
	assert(mpNode != 0);
	return Iterator(mpParser, GetChildren(), _childT);
}{{BI_NEWLINE}}

{{namespace}}::Iterator {{namespace}}::Iterator::GetNext({{namespace}}::SymbolType _childT)
{
	assert(mpNode != 0);
	return Iterator(*this, _childT);
}{{BI_NEWLINE}}

void {{namespace}}::Iterator::Print(std::ostream& _os, int _tabs, int _maxLineSize)
{
	mpParser->Print(_os, mType, mpNode, _tabs, _maxLineSize);
}{{BI_NEWLINE}}

{{namespace}}::Iterator::Iterator(boost::shared_ptr<{{namespace}}::Parser> _pParser, boost::shared_ptr<{{namespace}}::Symbols> _pSiblings, {{namespace}}::SymbolType _childType)
: mType(_childType)
, mpSiblings(_pSiblings)
, miCurrent(_pSiblings->begin())
,	mpParser(_pParser)
{
	SkipSiblingsWithWrongType(_childType);
}{{BI_NEWLINE}}

{{namespace}}::Iterator::Iterator(const {{namespace}}::Iterator& _iOther, {{namespace}}::SymbolType _childType)
: mType(_childType)
,	mpSiblings(_iOther.mpSiblings)
,	miCurrent(_iOther.miCurrent)
,	mpParser(_iOther.mpParser)
{
	SkipSiblingsWithWrongType(_childType);
}{{BI_NEWLINE}}

void {{namespace}}::Iterator::GoToNext({{namespace}}::SymbolType _childType)
{
	++miCurrent;
	SkipSiblingsWithWrongType(_childType);
}{{BI_NEWLINE}}

void {{namespace}}::Iterator::SkipSiblingsWithWrongType({{namespace}}::SymbolType _childType)
{
	Symbols::iterator iEnd = mpSiblings->end();
	while (miCurrent != iEnd && miCurrent->first != _childType)
		++miCurrent;
	mpNode = (miCurrent != iEnd) ? miCurrent->second : 0 ;
}{{BI_NEWLINE}}

boost::shared_ptr<{{namespace}}::Symbols> {{namespace}}::Iterator::GetChildren()
{
	if (!mpChildren)
	{
		Symbols children;
		int complexity = 0;
		mpParser->Traverse(mType, mpNode, children, complexity);
		mpChildren.reset(new Symbols);
		mpChildren->swap(children);			
	}
	
	return mpChildren;
}{{BI_NEWLINE}}

std::ostream& {{namespace}}::operator<<(std::ostream& _os, const {{namespace}}::Iterator& _i)
{
	if (_i)
	{
		for (const char *p = _i.Begin(), *pEnd = _i.End(); p != pEnd; ++p)
			_os.put(*p);
	}
	return _os;
}{{BI_NEWLINE}}
