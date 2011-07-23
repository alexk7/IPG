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
		{{#isNode}}
		case SymbolType_{{name}}: return "{{name}}";
		{{/isNode}}
		{{/def}}
		default: throw std::runtime_error(str(boost::format("Invalid Symbol Type: %1%") % _type));
	}
}{{BI_NEWLINE}}

bool {{namespace}}::Parser::Parse(SymbolType _type, const char*& p)
{
	const char* pBegin = p;
	bool r = true;
	char c;
	switch (_type)
	{
		{{#def}}
		{{#isNode}}
		case SymbolType_{{name}}:
		{
			if (fail[SymbolType_{{name}}].count(pBegin))
				return false;
			EndMap::iterator i = end[SymbolType_{{name}}].find(pBegin);
			if (i != end[SymbolType_{{name}}].end())
			{
				p = i->second;
				return true;
			}
			{{>parseCode}}
			if (r)
				end[SymbolType_{{name}}][pBegin] = p;
			else
				fail[SymbolType_{{name}}].insert(pBegin);
			return r;
		}{{BI_NEWLINE}}
		{{/isNode}}
		{{/def}}
		
		default:
			assert(false);
			return false;
	}
}{{BI_NEWLINE}}

bool {{namespace}}::Parser::Traverse({{namespace}}::SymbolType _type, const char*& p, {{namespace}}::Symbols& v)
{
	const char* pBegin = p;
	bool r = true;
	char c;
	switch (_type)
	{
		{{#def}}
		{{#isNode}}
		case SymbolType_{{name}}:
		{
			if (fail[SymbolType_{{name}}].count(pBegin))
				return false;
			{{>traverseCode}}
			if (r)
				end[SymbolType_{{name}}][pBegin] = p;
			else
				fail[SymbolType_{{name}}].insert(pBegin);
			return r;
		}{{BI_NEWLINE}}
		{{/isNode}}
		{{/def}}
			
		default:
			assert(false);
			return false;
	}
}{{BI_NEWLINE}}

void {{namespace}}::Parser::Print(std::ostream& _os, {{namespace}}::SymbolType _type, const char* _pNode, int _tabs, int _maxLineSize)
{
	Symbols children;
	const char* pEnd = _pNode;
	Traverse(_type, pEnd, children);
	//std::cout << children.size() << "\n";
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
		_os << "Memo Count:\n";
		for (size_t k = 0; k < SymbolTypeCount; ++k)
			_os << boost::format("%1% %|20t|End: %2% %|40t|Fail: %3%\n") % SymbolName(SymbolType(k)) % end[k].size() % fail[k].size();
	}
//*/
}{{BI_NEWLINE}}

bool {{namespace}}::Parser::Visit({{namespace}}::SymbolType _type, const char*& _p, {{namespace}}::Symbols& _v)
{
	const char* pBegin = _p;
	bool r = Parse(_type, _p);
	if (r)
		_v.push_back(Symbol(_type, pBegin));
	return r;
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
	const char* p = mpNode;
	mpParser->Parse(mType, p);
	return p;
}{{BI_NEWLINE}}

{{namespace}}::Iterator {{namespace}}::Iterator::GetChild({{namespace}}::SymbolType _childT) const
{
	assert(mpNode != 0);
	return Iterator(mpParser, GetChildren(), _childT);
}{{BI_NEWLINE}}

{{namespace}}::Iterator {{namespace}}::Iterator::GetNext({{namespace}}::SymbolType _childT) const
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

boost::shared_ptr<{{namespace}}::Symbols> {{namespace}}::Iterator::GetChildren() const
{
	if (!mpChildren)
	{
		Symbols children;
		const char* p = mpNode;
		mpParser->Traverse(mType, p, children);
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
