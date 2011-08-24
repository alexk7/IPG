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

	typedef std::pair<SymbolType, const char*> Symbol;
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
		FailSet fail[SymbolTypeCount];
		
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
		bool IsA(SymbolType _type) const { return mpSiblings && mi->first == _type; }
		operator bool() const { return mpSiblings; }
		
		Iterator(boost::shared_ptr<Parser> _pParser, SymbolType _type, const char* _p) : mpParser(_pParser)
		{
			Symbols children;
			mpEnd = _p;
			if (_pParser->Traverse(_type, mpEnd, children))
			{
				mpSiblings.reset(new Symbols(1, Symbol(_type, _p)));
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
				mpEnd = mi->second;
				bool r = mpParser->Traverse(mi->first, mpEnd, children);
				assert(r);
				if (!children.empty())
				{
					mpChildren.reset(new Symbols);
					mpChildren->swap(children);
				}
			}
			return *this;
		}

		const char* Begin() const
		{
			assert(mpSiblings && mi != mpSiblings->end());
			return mi->second;
		}
		
		const char* End() const
		{
			assert(mpSiblings && mi != mpSiblings->end());
			return mpEnd;
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
			mpParser->Print(_os, mi->first, mi->second, _tabs, _maxLineSize);
		}
	
	private:
		Iterator(boost::shared_ptr<Parser> _pParser, boost::shared_ptr<Symbols> _pSiblings) : mpParser(_pParser), mpSiblings(_pSiblings)
		{
			if (_pSiblings)
			{
				mi = _pSiblings->begin();
				mpEnd = mi->second;
				Symbols children;
				bool r =_pParser->Traverse(mi->first, mpEnd, children);
				assert(r);
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
		const char* mpEnd;
	};{{BI_NEWLINE}}

	std::ostream& operator<<(std::ostream& _os, const Iterator& _i);
}