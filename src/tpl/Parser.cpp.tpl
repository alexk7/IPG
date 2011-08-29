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

using namespace std;
using namespace std::tr1;
using namespace boost;
using namespace {{namespace}};

namespace
{
	typedef unordered_map<const char*, const char*> EndMap;
	typedef unordered_set<const char*> FailSet;{{BI_NEWLINE}}

	struct EscapeChar
	{
		EscapeChar(char _c);
		char c;
	};{{BI_NEWLINE}}
	
	EscapeChar::EscapeChar(char _c) : c(_c)
	{
	}{{BI_NEWLINE}}
	
	ostream& operator<<(ostream& _os, EscapeChar _e)
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
	}{{BI_NEWLINE}}
	
	bool Parse(Context& _ctx, SymbolType _type, const char*& p);{{BI_NEWLINE}}
	
	{{#def}}
	{{#isNode}}
	bool Parse_{{name}}(Context& _ctx, const char*& p);
	{{/isNode}}
	{{/def}}
}{{BI_NEWLINE}}

const char* {{namespace}}::SymbolName(SymbolType _type)
{
	switch (_type)
	{
		{{#def}}
		{{#isNode}}
		case SymbolType_{{name}}: return "{{name}}";
		{{/isNode}}
		{{/def}}
		default: throw runtime_error(str(format("Invalid Symbol Type: %1%") % _type));
	}
}{{BI_NEWLINE}}

class {{namespace}}::Context
{
public:
	bool Visit(SymbolType _type, const char*& _p, vector<Symbol>& _v);
	EndMap end[SymbolTypeCount];
	FailSet fail[SymbolTypeCount];{{BI_NEWLINE}}
};

bool {{namespace}}::Context::Visit(SymbolType _type, const char*& _p, vector<Symbol>& _v)
{
	const char* pBegin = _p;
	bool r = Parse(*this, _type, _p);
	if (r)
	{
		Symbol symbol = { _type, _p - pBegin, pBegin };
		_v.push_back(symbol);
	}
	return r;
}{{BI_NEWLINE}}

namespace
{
	bool Parse(Context& _ctx, SymbolType _type, const char*& p)
	{
		const char* pBegin = p;
		if (_ctx.fail[_type].count(pBegin))
			return false;
		EndMap::iterator i = _ctx.end[_type].find(pBegin);
		if (i != _ctx.end[_type].end())
		{
			p = i->second;
			return true;
		}{{BI_NEWLINE}}
		
		bool r = true;
		switch (_type)
		{
			{{#def}}
			{{#isNode}}
			case SymbolType_{{name}}: r = Parse_{{name}}(_ctx, p); break;
			{{/isNode}}
			{{/def}}
			default:
				assert(false);
				return false;
		}{{BI_NEWLINE}}
		
		if (r)
			_ctx.end[_type][pBegin] = p;
		else
			_ctx.fail[_type].insert(pBegin);
		return r;
	}{{BI_NEWLINE}}
	
	{{#def}}
	{{#isNode}}
	bool Parse_{{name}}(Context& _ctx, const char*& p)
	{
		bool r = true;
		char c;
		{{>parseCode}}
		return r;
	}{{BI_NEWLINE}}

	{{/isNode}}
	{{/def}}
	
	bool GetChildren(Context& _ctx, SymbolType _type, const char*& p, vector<Symbol>& v)
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
				if (_ctx.fail[SymbolType_{{name}}].count(pBegin))
					return false;
				{{>traverseCode}}
				if (r)
					_ctx.end[SymbolType_{{name}}][pBegin] = p;
				else
					_ctx.fail[SymbolType_{{name}}].insert(pBegin);
				return r;
			}{{BI_NEWLINE}}
			{{/isNode}}
			{{/def}}
				
			default:
				assert(false);
				return false;
		}
	}
	
	void DebugPrint(ostream& _os, Context& _ctx, SymbolType _type, const char* _pNode, int _tabs, int _maxLineSize)
	{
		vector<Symbol> children;
		const char* pEnd = _pNode;
		if (!GetChildren(_ctx, _type, pEnd, children))
			throw runtime_error(str(format("Parsing Failed for \"%1%\"") % SymbolName(_type)));{{BI_NEWLINE}}

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
		
		for (vector<Symbol>::iterator i = children.begin(), iEnd = children.end(); i != iEnd; ++i)
			DebugPrint(_os, _ctx, i->type, i->value, _tabs + 1, _maxLineSize);

	//*
		if (_tabs == 0)
		{
			_os << "Memo Count:\n";
			for (size_t k = 0; k < SymbolTypeCount; ++k)
				_os << boost::format("%1% %|20t|End: %2% %|40t|Fail: %3%\n") % SymbolName(SymbolType(k)) % _ctx.end[k].size() % _ctx.fail[k].size();
		}
	//*/
	}
}{{BI_NEWLINE}}

{{namespace}}::Iterator::Iterator()
{
}{{BI_NEWLINE}}

Iterator& {{namespace}}::Iterator::operator++()
{
	assert(mpSiblings);
	if (++mi == mpSiblings->end())
		mpSiblings.reset();
	return *this;
}{{BI_NEWLINE}}

const Symbol& {{namespace}}::Iterator::operator*() const
{
	static const Symbol invalidSymbol = { SymbolTypeInvalid };
	if (mpSiblings)
		return *mi;
	else
		return invalidSymbol;
}{{BI_NEWLINE}}

const Symbol* {{namespace}}::Iterator::operator->() const
{
	return &**this;
}{{BI_NEWLINE}}

{{namespace}}::Iterator::Iterator(shared_ptr<Context> _pContext, shared_ptr< vector<Symbol> > _pSiblings)
: mpContext(_pContext)
, mpSiblings(_pSiblings)
{
	if (_pSiblings)
		mi = _pSiblings->begin();
}{{BI_NEWLINE}}

Iterator {{namespace}}::Traverse(SymbolType _type, const char* _text)
{
	shared_ptr<Context> pContext(new Context);
	const char* p = _text;
	if (Parse(*pContext, _type, p))
	{
		Symbol symbol = { _type, p - _text, _text };
		shared_ptr< vector<Symbol> > pSymbols(new vector<Symbol>(1, symbol));
		return Iterator(pContext, pSymbols);
	}
	return Iterator();
}{{BI_NEWLINE}}

Iterator {{namespace}}::Traverse(const Iterator& _iParent)
{
	if (_iParent.mpSiblings)
	{
		const Symbol& symbol = *_iParent.mi;
		const char* p = symbol.value;
		vector<Symbol> children;
		bool r = GetChildren(*_iParent.mpContext, symbol.type, p, children);
		assert(r && p == symbol.value + symbol.length);
		boost::shared_ptr< vector<Symbol> > pChildren;
		if (!children.empty())
		{
			pChildren.reset(new vector<Symbol>);
			pChildren->swap(children);
		}
		return Iterator(_iParent.mpContext, pChildren);
	}
	return Iterator();
}{{BI_NEWLINE}}

void {{namespace}}::DebugPrint(ostream& _os, const Iterator& _i, int _tabs, int _maxLineSize)
{
	if (_i.mpSiblings)
		::DebugPrint(_os, *_i.mpContext, _i.mi->type, _i.mi->value, _tabs, _maxLineSize);
}{{BI_NEWLINE}}
