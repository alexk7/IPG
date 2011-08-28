#ifndef BOOTSTRAP_TEST
#include <vector>
#include <boost/shared_ptr.hpp>
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
	};{{BI_NEWLINE}}
	
	typedef std::vector<Symbol> Symbols;
		
	const char* SymbolName(SymbolType _type);{{BI_NEWLINE}}

	class Context;
	
	class Iterator
	{
	public:
		friend Iterator Parse(SymbolType _type, const char* _text);

		Iterator();
		Iterator& operator++();
		const Symbol& operator*() const;
		const Symbol* operator->() const;
		Iterator GetChild() const;
		void Print(std::ostream& _os, int _tabs = 0, int _maxLineSize = 100);
		
	private:
		Iterator(boost::shared_ptr<Context> _pParser, boost::shared_ptr<Symbols> _pSiblings);
		
		typedef std::vector<Symbol> Symbols;
		
		boost::shared_ptr<Context> mpContext;
		boost::shared_ptr<Symbols> mpSiblings;
		Symbols::iterator mi;
	};
	
	Iterator Parse(SymbolType _type, const char* _text);{{BI_NEWLINE}}
}{{BI_NEWLINE}}
