// This file was automatically generated by IPG on Mon Jul 18 19:08:44 2011
// (from /Users/alexk7/ipg/build/xcode/../../src/peg/peg.peg)
// DO NOT EDIT!
#ifndef PEGPARSER_H
#define PEGPARSER_H

#include <vector>
#include <map>

namespace PEGParser
{
	enum SymbolType
	{
		SymbolType_AND = 1,
		SymbolType_CLOSE = 2,
		SymbolType_Char = 3,
		SymbolType_Class = 4,
		SymbolType_Comment = 5,
		SymbolType_DOT = 6,
		SymbolType_Definition = 7,
		SymbolType_EndOfFile = 8,
		SymbolType_EndOfLine = 9,
		SymbolType_Expression = 10,
		SymbolType_Grammar = 11,
		SymbolType_Identifier = 12,
		SymbolType_LEFTARROW = 13,
		SymbolType_Literal = 14,
		SymbolType_NOT = 15,
		SymbolType_OPEN = 16,
		SymbolType_PLUS = 17,
		SymbolType_Prefix = 18,
		SymbolType_Primary = 19,
		SymbolType_QUESTION = 20,
		SymbolType_Range = 21,
		SymbolType_SLASH = 22,
		SymbolType_STAR = 23,
		SymbolType_Sequence = 24,
		SymbolType_Space = 25,
		SymbolType_Spacing = 26,
		SymbolType_Suffix = 27,
	};

	struct Node;
	typedef std::map<SymbolType, Node*> SymbolTypeToPtr;

	struct Node
	{
		char value;
		SymbolTypeToPtr end;
	};

	typedef std::pair<SymbolType, Node*> PTNodeChild;
	typedef std::vector<PTNodeChild> PTNodeChildren;

	Node* Parse(SymbolType _type, Node* _symbol);
	Node* Traverse(SymbolType _type, Node* _symbol, PTNodeChildren& _children);
	const char* SymbolName(SymbolType _type);
	void Print(std::ostream& _os, SymbolType _type, Node* _pNode, int _tabs = 0, int _maxLineSize = 100);
}

#endif
