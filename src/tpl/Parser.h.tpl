#ifndef BOOTSTRAP_TEST
#include <vector>
#include <bitset>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
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
	typedef std::bitset<SymbolTypeCount> SymbolTypeSet;
	typedef std::pair<SymbolTypeSet, const char*> MemoValue;
	typedef boost::unordered_multimap<const char*, MemoValue> MemoMap;{{BI_NEWLINE}}
		
	const char* SymbolName(SymbolType _type);{{BI_NEWLINE}}

	class Parser
	{
	public:
		const char* Parse(SymbolType _type, const char* _symbol);
		const char* Traverse(SymbolType _type, const char* _symbol, Symbols& _children);
		void Print(std::ostream& _os, SymbolType _type, const char* _pNode, int _tabs = 0, int _maxLineSize = 100);{{BI_NEWLINE}}
		
	private:
		const char** GetEnd(SymbolType _type, const char* _pBegin);
		void SetEnd(SymbolType _type, const char* _pBegin, const char* _pEnd);
		const char* Visit(SymbolType _type, const char* _p, Symbols& _v);
		MemoMap memoMap;
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