#include <vector>
#include <map>{{BI_NEWLINE}}

namespace {{name}}
{
	enum SymbolType
	{
		{{#def}}
		SymbolType_{{name}} = {{value}},
		{{/def}}
	};{{BI_NEWLINE}}

	struct Node;
	typedef std::map<SymbolType, Node*> SymbolTypeToPtr;{{BI_NEWLINE}}

	struct Node
	{
		char value;
		SymbolTypeToPtr end;
	};{{BI_NEWLINE}}
	
	typedef std::pair<SymbolType, Node*> PTNodeChild;
	typedef std::vector<PTNodeChild> PTNodeChildren;{{BI_NEWLINE}}

	Node* Parse(SymbolType _type, Node* _symbol);
	Node* Traverse(SymbolType _type, Node* _symbol, PTNodeChildren& _children);
	const char* SymbolName(SymbolType _type);
	void Print(std::ostream& _os, SymbolType _type, Node* _pNode, int _tabs = 0, int _maxLineSize = 100);
}