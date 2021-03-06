#ifndef BOOTSTRAP_TEST
#include <vector>
#include <boost/shared_ptr.hpp>
//#include <boost/cstdint.hpp>
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
    
    typedef char Char;
    typedef const char* CharItr;{{BI_NEWLINE}}
    //typedef boost::uint32_t Char;
    //typedef const boost::uint32_t* CharItr;{{BI_NEWLINE}}

	struct Symbol
	{
		SymbolType type;
		size_t length;
		CharItr value;
        bool success;
	};{{BI_NEWLINE}}
	
	const char*  SymbolName(SymbolType _type);{{BI_NEWLINE}}
	
	class Context;{{BI_NEWLINE}}
	
	class Iterator
	{
	public:
		friend Iterator Traverse(SymbolType _type, CharItr _text);
		friend Iterator Traverse(const Iterator& _iParent);
		friend void DebugPrint(std::ostream& _os, const Iterator& _i, int _tabs, int _maxLineSize);{{BI_NEWLINE}}

		Iterator();
		Iterator& operator++();
		const Symbol& operator*() const;
		const Symbol* operator->() const;{{BI_NEWLINE}}
		
	private:
		Iterator(boost::shared_ptr<Context> _pContext, boost::shared_ptr< std::vector<Symbol> > _pSiblings);{{BI_NEWLINE}}
		
		boost::shared_ptr<Context> mpContext;
		boost::shared_ptr< std::vector<Symbol> > mpSiblings;
		std::vector<Symbol>::iterator mi;
	};{{BI_NEWLINE}}
	
	Iterator Traverse(SymbolType _type, CharItr _text);
	Iterator Traverse(const Iterator& _iParent);
	void DebugPrint(std::ostream& _os, const Iterator& _i, int _tabs = 0, int _maxLineSize = 100);
}
