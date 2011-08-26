#ifndef BOOTSTRAP_TEST
#include <vector>
#include <bitset>
#include <boost/shared_ptr.hpp>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#endif{{BI_NEWLINE}}

namespace {{namespace}}
{
	enum SymbolType
	{
		{{#def}}
		{{#isNode}}
		SymbolType_{{name}},
		{{/isNode}}
		{{/def}}
		SymbolTypeInvalid,
		SymbolTypeCount = SymbolTypeInvalid
	};{{BI_NEWLINE}}

	struct Symbol
	{
		SymbolType type;
		size_t length;
		const char* value;
	};
	
	typedef std::vector<Symbol> Symbols;
	typedef std::tr1::unordered_map<const char*, const char*> EndMap;
	typedef std::tr1::unordered_set<const char*> FailSet;
	{{BI_NEWLINE}}
		
	const char* SymbolName(SymbolType _type);{{BI_NEWLINE}}

	class Parser
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
	};{{BI_NEWLINE}}
	
	class Iterator
	{
	public:
		Iterator() {}
		bool IsA(SymbolType _type) const { return mpSiblings && mi->type == _type; }
		operator bool() const { return mpSiblings; }
		
		Iterator(boost::shared_ptr<Parser> _pParser, SymbolType _type, const char* _p) : mpParser(_pParser)
		{
			Symbols children;
			const char* pEnd = _p;
			if (_pParser->Traverse(_type, pEnd, children))
			{
				Symbol symbol = { _type, pEnd - _p, _p };
				mpSiblings.reset(new Symbols(1, symbol));
				mi = mpSiblings->begin();
				if (!children.empty())
				{
					mpChildren.reset(new Symbols);
					mpChildren->swap(children);
				}
			}
		}
		
		Iterator& operator++()
		{
			assert(mpSiblings && mi != mpSiblings->end());
			mpChildren.reset();
			if (++mi == mpSiblings->end())
			{
				mpSiblings.reset();
			}
			else
			{
				Symbols children;
				const char* pEnd = mi->value;
				bool r = mpParser->Traverse(mi->type, pEnd, children);
				assert(r && pEnd == mi->value + mi->length);
				if (!children.empty())
				{
					mpChildren.reset(new Symbols);
					mpChildren->swap(children);
				}
			}
			return *this;
		}
		
		const Symbol& operator*() const
		{
			static Symbol invalidSymbol = { SymbolTypeInvalid };
			if (mpSiblings && mi != mpSiblings->end())
				return *mi;
			else
				return invalidSymbol;
		}
		
		const Symbol* operator->() const
		{
			return &**this;
		}
		
		Iterator GetChild() const
		{
			return Iterator(mpParser, mpChildren);
		}
		
		Iterator GetChild(SymbolType _type) const
		{
			Iterator i = GetChild();
			assert(i.IsA(_type));
			return i;
		}
		
		void Print(std::ostream& _os, int _tabs = 0, int _maxLineSize = 100)
		{
			assert(mpSiblings && mi != mpSiblings->end());
			mpParser->Print(_os, mi->type, mi->value, _tabs, _maxLineSize);
		}
	
	private:
		Iterator(boost::shared_ptr<Parser> _pParser, boost::shared_ptr<Symbols> _pSiblings) : mpParser(_pParser), mpSiblings(_pSiblings)
		{
			if (_pSiblings)
			{
				mi = _pSiblings->begin();
				const char* pEnd = mi->value;
				Symbols children;
				bool r =_pParser->Traverse(mi->type, pEnd, children);
				assert(r && pEnd == mi->value + mi->length);
				if (!children.empty())
				{
					mpChildren.reset(new Symbols);
					mpChildren->swap(children);
				}
			}
		}
		
		boost::shared_ptr<Parser> mpParser;
		boost::shared_ptr<Symbols> mpSiblings, mpChildren;
		Symbols::iterator mi;
	};{{BI_NEWLINE}}
}