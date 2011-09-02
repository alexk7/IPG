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
using namespace {{namespace}};{{BI_NEWLINE}}

namespace
{
	struct Memo
	{
		unordered_map<const char*, const char*> end;
		unordered_set<const char*> fail;
	};

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
	
	bool ParseSymbol(Context&, SymbolType, const char*&);{{BI_NEWLINE}}
	
	typedef bool ParseFn(Context&, const char*&);
	ParseFn {{#def}}Parse_{{name}}{{#def_separator}}, {{/def_separator}}{{/def}};{{BI_NEWLINE}}

	typedef bool TraverseFn(Context&, const char*&, vector<Symbol>&);
	TraverseFn {{#def}}Traverse_{{name}}{{#def_separator}}, {{/def_separator}}{{/def}};
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
	Memo memos[SymbolTypeCount];
};{{BI_NEWLINE}}

namespace
{
	bool Visit(Context& _ctx, SymbolType _type, const char*& _p, vector<Symbol>& _v)
	{
		const char* pBegin = _p;
		bool r = ParseSymbol(_ctx, _type, _p);
		if (r)
		{
			Symbol symbol = { _type, _p - pBegin, pBegin };
			_v.push_back(symbol);
		}
		return r;
	}{{BI_NEWLINE}}

	bool ParseMemoized(Context& _ctx, Memo& _memo, ParseFn* _parseFn, const char*& p)
	{
		if (_memo.fail.count(p))
			return false;{{BI_NEWLINE}}
			
		unordered_map<const char*, const char*>::iterator i = _memo.end.find(p);
		if (i != _memo.end.end())
		{
			p = i->second;
			return true;
		}{{BI_NEWLINE}}
		
		const char* pBegin = p;
		if (_parseFn(_ctx, p))
		{
			_memo.end[pBegin] = p;
			return true;
		}{{BI_NEWLINE}}
		
		_memo.fail.insert(pBegin);
		return false;
	}
		
	bool ParseSymbol(Context& _ctx, SymbolType _type, const char*& p)
	{
		assert(_type >= 0 && _type < SymbolTypeCount);
		switch (_type)
		{
			{{#def}}
			{{#isNode}}
			case SymbolType_{{name}}: return ParseMemoized(_ctx, _ctx.memos[_type], Parse_{{name}}, p);
			{{/isNode}}
			{{/def}}
			default:
				assert(false);
				return false;
		}
	}{{BI_NEWLINE}}
	
	{{#def}}
	bool Parse_{{name}}(Context& _ctx, const char*& p)
	{
		bool r = true;
		{{>parseCode}}
		return r;
	}{{BI_NEWLINE}}

	{{/def}}
	
	bool TraverseSymbol(Context& _ctx, SymbolType _type, const char*& p, vector<Symbol>& v)
	{
		const char* pBegin = p;
		if (_ctx.memos[_type].fail.count(pBegin))
			return false;{{BI_NEWLINE}}
			
		bool r = true;
		switch (_type)
		{
			{{#def}}
			{{#isNode}}
			case SymbolType_{{name}}: r = Traverse_{{name}}(_ctx, p, v); break;
			{{/isNode}}
			{{/def}}
			default:
				assert(false);
				return false;
		}{{BI_NEWLINE}}
		
		if (r)
			_ctx.memos[_type].end[pBegin] = p;
		else
			_ctx.memos[_type].fail.insert(pBegin);
		return r;
	}

	{{#def}}
	bool Traverse_{{name}}(Context& _ctx, const char*& p, vector<Symbol>& v)
	{
		bool r = true;
		{{>traverseCode}}
		return r;
	}{{BI_NEWLINE}}

	{{/def}}
	
	void DebugPrint(ostream& _os, Context& _ctx, SymbolType _type, const char* _pNode, int _tabs, int _maxLineSize)
	{
		vector<Symbol> children;
		const char* pEnd = _pNode;
		if (!TraverseSymbol(_ctx, _type, pEnd, children))
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
				_os << boost::format("%1% %|20t|End: %2% %|40t|Fail: %3%\n") % SymbolName(SymbolType(k)) % _ctx.memos[k].end.size() % _ctx.memos[k].fail.size();
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
	if (ParseSymbol(*pContext, _type, p))
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
		bool r = TraverseSymbol(*_iParent.mpContext, symbol.type, p, children);
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
