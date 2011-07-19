#include <cassert>
#include <stdexcept>
#include <ostream>
#include <sstream>
#include <iomanip>
{{header}}

namespace
{
	typedef {{namespace}}::SymbolTypeToPtr::value_type Memo;
	typedef {{namespace}}::SymbolTypeToPtr::iterator MemoIterator;
	typedef std::pair<{{namespace}}::SymbolTypeToPtr::iterator, bool> MemoInsertResult;{{BI_NEWLINE}}
	
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
	}
}{{BI_NEWLINE}}

{{namespace}}::Node* {{namespace}}::Parser::Parse(SymbolType _type, {{namespace}}::Node* _symbol)
{
	switch (_type)
	{
		{{#def}}
		case SymbolType_{{name}}: return Parse_{{name}}(_symbol);
		{{/def}}
	}
	return 0;
}{{BI_NEWLINE}}

{{namespace}}::Node* {{namespace}}::Parser::Traverse({{namespace}}::SymbolType _type, {{namespace}}::Node* _symbol, {{namespace}}::PTNodeChildren& _children)
{
	switch (_type)
	{
		{{#def}}
		{{#isInternal}}
		case SymbolType_{{name}}: return Traverse_{{name}}(_symbol, _children);
		{{/isInternal}}
		{{/def}}
		{{#def}}
		{{#isLeaf}}
		case SymbolType_{{name}}:
		{{/isLeaf}}
		{{/def}}
		default:
			return Parse(_type, _symbol);
	}
}{{BI_NEWLINE}}

void {{namespace}}::Parser::Print(std::ostream& _os, {{namespace}}::SymbolType _type, {{namespace}}::Node* _pNode, int _tabs, int _maxLineSize)
{
	PTNodeChildren children;
	Node* pEnd = Traverse(_type, _pNode, children);
	if (!pEnd)
	{
		std::ostringstream oss;
		oss << "Parsing Failed for \"" << SymbolName(_type) << "\"";
		throw std::runtime_error(oss.str());
	}{{BI_NEWLINE}}

	int tabCount = _tabs;
	while (tabCount--)
		_os << "    ";{{BI_NEWLINE}}
	
	_os << SymbolName(_type) << ": \"";{{BI_NEWLINE}}

	size_t lineSize = 0;
	for (Node* p = _pNode; p != pEnd; ++p)
	{
		_os << EscapeChar(p->value);
		if (++lineSize >= _maxLineSize)
		{
			_os << "...";
			break;
		}
	}{{BI_NEWLINE}}
	
	_os << "\"\n";{{BI_NEWLINE}}
	
	for (PTNodeChildren::iterator i = children.begin(), iEnd = children.end(); i != iEnd; ++i)
		Print(_os, i->first, i->second, _tabs + 1, _maxLineSize);
}{{BI_NEWLINE}}

{{namespace}}::Node* {{namespace}}::Parser::Visit({{namespace}}::SymbolType _type, {{namespace}}::Node* _p, {{namespace}}::PTNodeChildren& _v)
{
	Node* pEnd = Parse(_type, _p);
	if (pEnd)
		_v.push_back(PTNodeChild(_type, _p));
	return pEnd;
}{{BI_NEWLINE}}
	
{{#def}}
{{namespace}}::Node* {{namespace}}::Parser::Parse_{{name}}({{namespace}}::Node* p)
{
	{{#isMemoized}}
	MemoInsertResult r = p->end.insert(Memo(SymbolType_{{name}}, 0));
	if (!r.second) return r.first->second;
	{{/isMemoized}}
	{{>parseCode}}
	{{#isMemoized}}
	r.first->second = p;
	{{/isMemoized}}
	return p;
}{{BI_NEWLINE}}
{{/def}}

{{#def}}
{{#isInternal}}
{{namespace}}::Node* {{namespace}}::Parser::Traverse_{{name}}({{namespace}}::Node* p, {{namespace}}::PTNodeChildren& v)
{
	{{>traverseCode}}
	{{#isMemoized}}
	b->end.insert(Memo(SymbolType_{{name}}, p));
	{{/isMemoized}}
	return p;
}{{BI_NEWLINE}}
{{/isInternal}}
{{/def}}

{{namespace}}::PTItr::PTItr({{namespace}}::SymbolType _type, {{namespace}}::Node* _pNode)
:	mType(_type)
,	mpNode(_pNode)
, mpSiblings(new PTNodeChildren)
, mpParser(new Parser)
{
	if (mpNode && End())
		mpSiblings->push_back(PTNodeChild(mType, mpNode));{{BI_NEWLINE}}
	miCurrent = mpSiblings->begin();
}{{BI_NEWLINE}}

{{namespace}}::PTItr::PTItr(const {{namespace}}::PTItr& _iOther)
: mType(_iOther.mType)
,	mpNode(_iOther.mpNode)
, mpSiblings(_iOther.mpSiblings)
, miCurrent(_iOther.miCurrent)
,	mpParser(_iOther.mpParser)
{
}{{BI_NEWLINE}}
	
{{namespace}}::PTItr::operator bool() const
{
	return mpNode != 0;
}{{BI_NEWLINE}}

{{namespace}}::PTItr& {{namespace}}::PTItr::operator++()
{
	mpChildren.reset();
	GoToNext(mType);
	return *this;
}{{BI_NEWLINE}}
	
{{namespace}}::SymbolType {{namespace}}::PTItr::GetType() const
{
	return mType;
}{{BI_NEWLINE}}

{{namespace}}::Node* {{namespace}}::PTItr::Begin() const
{
	assert(mpNode);
	return mpNode;
}{{BI_NEWLINE}}

{{namespace}}::Node* {{namespace}}::PTItr::End() const
{
	assert(mpNode);
	if (mType == SymbolType(0))
		return mpNode + 1;
	return mpParser->Parse(mType, mpNode);
}{{BI_NEWLINE}}

{{namespace}}::PTItr {{namespace}}::PTItr::GetChild({{namespace}}::SymbolType _childT)
{
	assert(mpNode != 0);
	if (_childT == SymbolType(0))
		return PTItr(SymbolType(0), mpNode);
	else
		return PTItr(mpParser, GetChildren(), _childT);
}{{BI_NEWLINE}}

{{namespace}}::PTItr {{namespace}}::PTItr::GetNext({{namespace}}::SymbolType _childT)
{
	assert(mpNode != 0);
	if (_childT == SymbolType(0))
		return PTItr(SymbolType(0), mpNode + 1);
	else
		return PTItr(*this, _childT);
}{{BI_NEWLINE}}

{{namespace}}::PTItr::PTItr(boost::shared_ptr<{{namespace}}::Parser> _pParser, boost::shared_ptr<{{namespace}}::PTNodeChildren> _pSiblings, {{namespace}}::SymbolType _childType)
: mType(_childType)
, mpSiblings(_pSiblings)
, miCurrent(_pSiblings->begin())
,	mpParser(_pParser)
{
	SkipSiblingsWithWrongType(_childType);
}{{BI_NEWLINE}}

{{namespace}}::PTItr::PTItr(const {{namespace}}::PTItr& _iOther, {{namespace}}::SymbolType _childType)
: mType(_childType)
,	mpSiblings(_iOther.mpSiblings)
,	miCurrent(_iOther.miCurrent)
,	mpParser(_iOther.mpParser)
{
	SkipSiblingsWithWrongType(_childType);
}{{BI_NEWLINE}}

void {{namespace}}::PTItr::GoToNext({{namespace}}::SymbolType _childType)
{
	if (mType == SymbolType(0))
	{
		++mpNode;
		if (mpNode->value == 0)
			mpNode = 0;
	}
	else
	{
		++miCurrent;
		SkipSiblingsWithWrongType(_childType);
	}
}{{BI_NEWLINE}}

void {{namespace}}::PTItr::SkipSiblingsWithWrongType({{namespace}}::SymbolType _childType)
{
	assert(_childType != SymbolType(0));
	PTNodeChildren::iterator iEnd = mpSiblings->end();
	while (miCurrent != iEnd && miCurrent->first != _childType)
		++miCurrent;
	mpNode = (miCurrent != iEnd) ? miCurrent->second : 0 ;
}{{BI_NEWLINE}}

boost::shared_ptr<{{namespace}}::PTNodeChildren> {{namespace}}::PTItr::GetChildren()
{
	if (!mpChildren)
	{
		PTNodeChildren children;
		mpParser->Traverse(mType, mpNode, children);
		mpChildren.reset(new PTNodeChildren);
		mpChildren->swap(children);			
	}
	
	return mpChildren;
}{{BI_NEWLINE}}

std::ostream& {{namespace}}::operator<<(std::ostream& _os, const {{namespace}}::PTItr& _i)
{
	if (_i)
	{
		for (const {{namespace}}::Node *p = _i.Begin(), *pEnd = _i.End(); p != pEnd; ++p)
			_os.put(p->value);
	}
	return _os;
}{{BI_NEWLINE}}
