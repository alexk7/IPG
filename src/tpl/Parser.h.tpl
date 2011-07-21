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
		SymbolType_{{name}} = {{value}},
		{{/def}}
		SymbolTypeCount
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
		const char* Parse(SymbolType _type, const char* _symbol, bool _memoize = true);
		const char* Traverse(SymbolType _type, const char* _symbol, Symbols& _children, bool _memoize = true);
		void Print(std::ostream& _os, SymbolType _type, const char* _pNode, int _tabs = 0, int _maxLineSize = 100);{{BI_NEWLINE}}
		
	private:
		const char* Visit(SymbolType _type, const char* _p, Symbols& _v);
		EndMap end[SymbolTypeCount];
		FailSet fail[SymbolTypeCount];
	};{{BI_NEWLINE}}
	
	class Iterator
	{
	public:
		Iterator(SymbolType _type, const char* _pNode = 0);
		Iterator(const Iterator& _iOther);
		operator bool() const;
		Iterator& operator++();
		SymbolType GetType() const;
		const char* Begin() const;
		const char* End() const;
		Iterator GetChild(SymbolType _childT);
		Iterator GetNext(SymbolType _childT);
		void Print(std::ostream& _os, int _tabs = 0, int _maxLineSize = 100);{{BI_NEWLINE}}
		
	private:
		Iterator(boost::shared_ptr<Parser> _pParser, boost::shared_ptr<Symbols> _pSiblings, SymbolType _childType);
		Iterator(const Iterator& _iOther, SymbolType _childType);
		void GoToNext(SymbolType _childType);
		void SkipSiblingsWithWrongType(SymbolType _childType);
		boost::shared_ptr<Symbols> GetChildren();{{BI_NEWLINE}}
		
		SymbolType mType;
		const char* mpNode;
		boost::shared_ptr<Symbols> mpSiblings;
		Symbols::iterator miCurrent;
		boost::shared_ptr<Symbols> mpChildren;
		boost::shared_ptr<Parser> mpParser;
	};{{BI_NEWLINE}}

	std::ostream& operator<<(std::ostream& _os, const Iterator& _i);
}