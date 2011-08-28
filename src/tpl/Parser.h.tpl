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
		
	const char* SymbolName(SymbolType _type);

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
	
	class Iterator;
	Iterator Parse(SymbolType _type, const char* _text);
	
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
		Iterator(boost::shared_ptr<Parser> _pParser, boost::shared_ptr<Symbols> _pSiblings);
		
		boost::shared_ptr<Parser> mpParser;
		boost::shared_ptr<Symbols> mpSiblings;
		Symbols::iterator mi;
	};
}