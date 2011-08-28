#ifndef BOOTSTRAP_TEST
#include <cassert>
#include <stdexcept>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <boost/format.hpp>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#endif{{BI_NEWLINE}}

{{header}}

namespace
{
	typedef std::tr1::unordered_map<const char*, const char*> EndMap;
	typedef std::tr1::unordered_set<const char*> FailSet;{{BI_NEWLINE}}

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
			case '\n': c = 'n'; break;
			case '\r': c = 'r'; break;
			case '\t': c = 't'; break;{{BI_NEWLINE}}
			
			case '\\':
			case '\'':
			case '\"':
				break;{{BI_NEWLINE}}
				
			default:
				_os.put(c);
				return _os;
		}{{BI_NEWLINE}}
		
		_os.put('\\');
		_os.put(c);
		return _os;
	}
}{{BI_NEWLINE}}

class {{namespace}}::Context
{
public:
	bool Parse(SymbolType _type, const char*& _p);
	bool Traverse(SymbolType _type, const char*& _p, Symbols& _children);
	void Print(std::ostream& _os, SymbolType _type, const char* _pNode, int _tabs = 0, int _maxLineSize = 100);{{BI_NEWLINE}}
	
private:
	bool Visit(SymbolType _type, const char*& _p, Symbols& _v);
	EndMap end[SymbolTypeCount];
	FailSet fail[SymbolTypeCount];{{BI_NEWLINE}}
	
	{{#def}}
	{{#isNode}}
	bool Parse_{{name}}(const char*& p);
	{{/isNode}}
	{{/def}}
};

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

{{#def}}
{{#isNode}}
bool {{namespace}}::Context::Parse_{{name}}(const char*& p)
{
	bool r = true;
	char c;
	{{>parseCode}}
	return r;
}{{BI_NEWLINE}}

{{/isNode}}
{{/def}}

bool {{namespace}}::Context::Parse(SymbolType _type, const char*& p)
{
	const char* pBegin = p;
	if (fail[_type].count(pBegin))
		return false;
	EndMap::iterator i = end[_type].find(pBegin);
	if (i != end[_type].end())
	{
		p = i->second;
		return true;
	}{{BI_NEWLINE}}
	
	bool r = true;
	switch (_type)
	{
		{{#def}}
		{{#isNode}}
		case SymbolType_{{name}}: r = Parse_{{name}}(p); break;
		{{/isNode}}
		{{/def}}
		default:
			assert(false);
			return false;
	}{{BI_NEWLINE}}
	
	if (r)
		end[_type][pBegin] = p;
	else
		fail[_type].insert(pBegin);
	return r;
}{{BI_NEWLINE}}

bool {{namespace}}::Context::Traverse({{namespace}}::SymbolType _type, const char*& p, {{namespace}}::Symbols& v)
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

void {{namespace}}::Context::Print(std::ostream& _os, {{namespace}}::SymbolType _type, const char* _pNode, int _tabs, int _maxLineSize)
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
		Print(_os, i->type, i->value, _tabs + 1, _maxLineSize);

//*
	if (_tabs == 0)
	{
		_os << "Memo Count:\n";
		for (size_t k = 0; k < SymbolTypeCount; ++k)
			_os << boost::format("%1% %|20t|End: %2% %|40t|Fail: %3%\n") % SymbolName(SymbolType(k)) % end[k].size() % fail[k].size();
	}
//*/
}{{BI_NEWLINE}}

bool {{namespace}}::Context::Visit({{namespace}}::SymbolType _type, const char*& _p, {{namespace}}::Symbols& _v)
{
	const char* pBegin = _p;
	bool r = Parse(_type, _p);
	if (r)
	{
		Symbol symbol = { _type, _p - pBegin, pBegin };
		_v.push_back(symbol);
	}
	return r;
}{{BI_NEWLINE}}

{{namespace}}::Iterator {{namespace}}::Parse({{namespace}}::SymbolType _type, const char* _text)
{
	boost::shared_ptr<Context> pContext(new Context);
	const char* p = _text;
	if (pContext->Parse(_type, p))
	{
		Symbol symbol = { _type, p - _text, _text };
		boost::shared_ptr<Symbols> pSymbols(new Symbols(1, symbol));
		return Iterator(pContext, pSymbols);
	}
	return Iterator();
}{{BI_NEWLINE}}

{{namespace}}::Iterator::Iterator()
{
}{{BI_NEWLINE}}

{{namespace}}::Iterator& {{namespace}}::Iterator::operator++()
{
	assert(mpSiblings);
	if (++mi == mpSiblings->end())
		mpSiblings.reset();
	return *this;
}{{BI_NEWLINE}}

const {{namespace}}::Symbol& {{namespace}}::Iterator::operator*() const
{
	static const Symbol invalidSymbol = { SymbolTypeInvalid };
	if (mpSiblings)
		return *mi;
	else
		return invalidSymbol;
}{{BI_NEWLINE}}

const {{namespace}}::Symbol* {{namespace}}::Iterator::operator->() const
{
	return &**this;
}{{BI_NEWLINE}}

{{namespace}}::Iterator {{namespace}}::Iterator::GetChild() const
{
	if (mpSiblings)
	{
		Symbols children;
		const char* p = mi->value;
		bool r = mpContext->Traverse(mi->type, p, children);
		assert(r && p == mi->value + mi->length);
		boost::shared_ptr<Symbols> pChildren;
		if (!children.empty())
		{
			pChildren.reset(new Symbols);
			pChildren->swap(children);
		}
		return Iterator(mpContext, pChildren);
	}
	return Iterator();
}{{BI_NEWLINE}}

void {{namespace}}::Iterator::Print(std::ostream& _os, int _tabs, int _maxLineSize)
{
	if (mpSiblings)
		mpContext->Print(_os, mi->type, mi->value, _tabs, _maxLineSize);
}{{BI_NEWLINE}}

{{namespace}}::Iterator::Iterator(boost::shared_ptr<Context> _pParser, boost::shared_ptr<Symbols> _pSiblings)
: mpContext(_pParser)
, mpSiblings(_pSiblings)
{
	if (_pSiblings)
		mi = _pSiblings->begin();
}{{BI_NEWLINE}}
