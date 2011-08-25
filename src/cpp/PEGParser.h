// This file was automatically generated by IPG on Thu Aug 25 11:51:23 2011
// (from /Users/alexk7/ipg/build/xcode/../../src/peg/peg.peg)
// DO NOT EDIT!
#ifndef PEGPARSER_H
#define PEGPARSER_H

#ifndef BOOTSTRAP_TEST
#include <vector>
#include <bitset>
#include <boost/shared_ptr.hpp>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#endif

namespace PEGParser
{
	enum SymbolType
	{
		SymbolType_CLASS,
		SymbolType_CLASS_1,
		SymbolType_Char,
		SymbolType_Definition,
		SymbolType_Expression,
		SymbolType_Expression_1,
		SymbolType_Grammar,
		SymbolType_Grammar_1,
		SymbolType_IDENTIFIER_1,
		SymbolType_Identifier,
		SymbolType_Item,
		SymbolType_Item_1,
		SymbolType_Item_2,
		SymbolType_LEFTARROW,
		SymbolType_LITERAL,
		SymbolType_LITERAL_1,
		SymbolType_Primary,
		SymbolType_Primary_1,
		SymbolType_Primary_2,
		SymbolType_Primary_3,
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
	};
	typedef std::vector<Symbol> Symbols;
	typedef std::tr1::unordered_map<const char*, const char*> EndMap;
	typedef std::tr1::unordered_set<const char*> FailSet;
	

	const char* SymbolName(SymbolType _type);

	class Parser
	{
	public:
		bool Parse(SymbolType _type, const char*& _p);
		bool Traverse(SymbolType _type, const char*& _p, Symbols& _children);
		void Print(std::ostream& _os, SymbolType _type, const char* _pNode, int _tabs = 0, int _maxLineSize = 100);

	private:
		bool Visit(SymbolType _type, const char*& _p, Symbols& _v);
		EndMap end[SymbolTypeCount];
		FailSet fail[SymbolTypeCount];

		bool Parse_CLASS(const char*& p);
		bool Parse_CLASS_1(const char*& p);
		bool Parse_Char(const char*& p);
		bool Parse_Definition(const char*& p);
		bool Parse_Expression(const char*& p);
		bool Parse_Expression_1(const char*& p);
		bool Parse_Grammar(const char*& p);
		bool Parse_Grammar_1(const char*& p);
		bool Parse_IDENTIFIER_1(const char*& p);
		bool Parse_Identifier(const char*& p);
		bool Parse_Item(const char*& p);
		bool Parse_Item_1(const char*& p);
		bool Parse_Item_2(const char*& p);
		bool Parse_LEFTARROW(const char*& p);
		bool Parse_LITERAL(const char*& p);
		bool Parse_LITERAL_1(const char*& p);
		bool Parse_Primary(const char*& p);
		bool Parse_Primary_1(const char*& p);
		bool Parse_Primary_2(const char*& p);
		bool Parse_Primary_3(const char*& p);
		bool Parse_Range(const char*& p);
		bool Parse_Sequence(const char*& p);
	};

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
			assert(mpSiblings && mi != mpSiblings->end());
			return *mi;
		}
		const Symbol* operator->() const
		{
			assert(mpSiblings && mi != mpSiblings->end());
			return mi.operator->();
		}
/*
		const char* Begin() const
		{
			assert(mpSiblings && mi != mpSiblings->end());
			return mi->value;
		}
		const char* End() const
		{
			assert(mpSiblings && mi != mpSiblings->end());
			return mi->value + mi->length;
		}
*/
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
	};

	std::ostream& operator<<(std::ostream& _os, const Iterator& _i);
}

#endif
