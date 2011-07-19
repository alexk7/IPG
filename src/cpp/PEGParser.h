// This file was automatically generated by IPG on Mon Jul 18 21:57:38 2011
// (from /Users/alexk7/ipg/build/xcode/../../src/peg/peg.peg)
// DO NOT EDIT!
#ifndef PEGPARSER_H
#define PEGPARSER_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

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
	typedef boost::unordered_map<SymbolType, Node*> SymbolTypeToPtr;

	struct Node
	{
		char value;
		SymbolTypeToPtr end;
	};

	typedef std::pair<SymbolType, Node*> PTNodeChild;
	typedef std::vector<PTNodeChild> PTNodeChildren;

	const char* SymbolName(SymbolType _type);

	class Parser
	{
	public:
		Node* Parse(SymbolType _type, Node* _symbol);
		Node* Traverse(SymbolType _type, Node* _symbol, PTNodeChildren& _children);
		void Print(std::ostream& _os, SymbolType _type, Node* _pNode, int _tabs = 0, int _maxLineSize = 100);

	private:
		Node* Visit(SymbolType _type, Node* _p, PTNodeChildren& _v);
		Node* Parse_AND(Node* p);
		Node* Parse_CLOSE(Node* p);
		Node* Parse_Char(Node* p);
		Node* Parse_Class(Node* p);
		Node* Parse_Comment(Node* p);
		Node* Parse_DOT(Node* p);
		Node* Parse_Definition(Node* p);
		Node* Parse_EndOfFile(Node* p);
		Node* Parse_EndOfLine(Node* p);
		Node* Parse_Expression(Node* p);
		Node* Parse_Grammar(Node* p);
		Node* Parse_Identifier(Node* p);
		Node* Parse_LEFTARROW(Node* p);
		Node* Parse_Literal(Node* p);
		Node* Parse_NOT(Node* p);
		Node* Parse_OPEN(Node* p);
		Node* Parse_PLUS(Node* p);
		Node* Parse_Prefix(Node* p);
		Node* Parse_Primary(Node* p);
		Node* Parse_QUESTION(Node* p);
		Node* Parse_Range(Node* p);
		Node* Parse_SLASH(Node* p);
		Node* Parse_STAR(Node* p);
		Node* Parse_Sequence(Node* p);
		Node* Parse_Space(Node* p);
		Node* Parse_Spacing(Node* p);
		Node* Parse_Suffix(Node* p);
		Node* Traverse_Class(Node* p, PTNodeChildren& v);
		Node* Traverse_Definition(Node* p, PTNodeChildren& v);
		Node* Traverse_Expression(Node* p, PTNodeChildren& v);
		Node* Traverse_Grammar(Node* p, PTNodeChildren& v);
		Node* Traverse_Literal(Node* p, PTNodeChildren& v);
		Node* Traverse_Prefix(Node* p, PTNodeChildren& v);
		Node* Traverse_Primary(Node* p, PTNodeChildren& v);
		Node* Traverse_Range(Node* p, PTNodeChildren& v);
		Node* Traverse_Sequence(Node* p, PTNodeChildren& v);
		Node* Traverse_Suffix(Node* p, PTNodeChildren& v);
	};

	class PTItr
	{
	public:
		PTItr(SymbolType _type, Node* _pNode = 0);
		PTItr(const PTItr& _iOther);
		operator bool() const;
		PTItr& operator++();
		SymbolType GetType() const;
		Node* Begin() const;
		Node* End() const;
		PTItr GetChild(SymbolType _childT);
		PTItr GetNext(SymbolType _childT);

	private:
		PTItr(boost::shared_ptr<Parser> _pParser, boost::shared_ptr<PTNodeChildren> _pSiblings, SymbolType _childType);
		PTItr(const PTItr& _iOther, SymbolType _childType);
		void GoToNext(SymbolType _childType);
		void SkipSiblingsWithWrongType(SymbolType _childType);
		boost::shared_ptr<PTNodeChildren> GetChildren();

		SymbolType mType;
		Node* mpNode;
		boost::shared_ptr<PTNodeChildren> mpSiblings;
		PTNodeChildren::iterator miCurrent;
		boost::shared_ptr<PTNodeChildren> mpChildren;
		boost::shared_ptr<Parser> mpParser;
	};

	std::ostream& operator<<(std::ostream& _os, const PTItr& _i);
}

#endif
