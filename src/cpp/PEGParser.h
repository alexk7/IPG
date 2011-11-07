// This file was automatically generated by IPG on Mon Nov  7 10:40:44 2011
// (from /Users/alexk7/Documents/IPG/build/xcode/../../src/peg/peg.peg)
// DO NOT EDIT!
#ifndef PEGPARSER_H
#define PEGPARSER_H

#ifndef BOOTSTRAP_TEST
#include <vector>
#include <boost/shared_ptr.hpp>
#endif

namespace PEGParser
{
	enum SymbolType
	{
		SymbolType_CLASS,
		SymbolType_Char,
		SymbolType_Definition,
		SymbolType_Expression,
		SymbolType_Grammar,
		SymbolType_Identifier,
		SymbolType_Item,
		SymbolType_LEFTARROW,
		SymbolType_LITERAL,
		SymbolType_Primary,
		SymbolType_Range,
		SymbolType_Sequence,
		SymbolTypeInvalid,
		SymbolTypeCount = SymbolTypeInvalid
	};

	struct Symbol
	{
		SymbolType type;
		size_t length;
		const char* value;
        bool success;
	};

	const char* SymbolName(SymbolType _type);

	class Context;

	class Iterator
	{
	public:
		friend Iterator Traverse(SymbolType _type, const char* _text);
		friend Iterator Traverse(const Iterator& _iParent);
		friend void DebugPrint(std::ostream& _os, const Iterator& _i, int _tabs, int _maxLineSize);

		Iterator();
		Iterator& operator++();
		const Symbol& operator*() const;
		const Symbol* operator->() const;

	private:
		Iterator(boost::shared_ptr<Context> _pContext, boost::shared_ptr< std::vector<Symbol> > _pSiblings);

		boost::shared_ptr<Context> mpContext;
		boost::shared_ptr< std::vector<Symbol> > mpSiblings;
		std::vector<Symbol>::iterator mi;
	};

	Iterator Traverse(SymbolType _type, const char* _text);
	Iterator Traverse(const Iterator& _iParent);
	void DebugPrint(std::ostream& _os, const Iterator& _i, int _tabs = 0, int _maxLineSize = 100);
}

#endif
