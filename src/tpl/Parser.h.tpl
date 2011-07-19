#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>{{BI_NEWLINE}}

namespace {{namespace}}
{
	enum SymbolType
	{
		{{#def}}
		SymbolType_{{name}} = {{value}},
		{{/def}}
	};{{BI_NEWLINE}}

	struct Node;
	typedef boost::unordered_map<SymbolType, Node*> SymbolTypeToPtr;{{BI_NEWLINE}}

	struct Node
	{
		char value;
		SymbolTypeToPtr end;
	};{{BI_NEWLINE}}
	
	typedef std::pair<SymbolType, Node*> PTNodeChild;
	typedef std::vector<PTNodeChild> PTNodeChildren;{{BI_NEWLINE}}
	
	const char* SymbolName(SymbolType _type);{{BI_NEWLINE}}

	class Parser
	{
	public:
		Node* Parse(SymbolType _type, Node* _symbol);
		Node* Traverse(SymbolType _type, Node* _symbol, PTNodeChildren& _children);
		void Print(std::ostream& _os, SymbolType _type, Node* _pNode, int _tabs = 0, int _maxLineSize = 100);{{BI_NEWLINE}}
		
	private:
		Node* Visit(SymbolType _type, Node* _p, PTNodeChildren& _v);
		{{#def}}
		Node* Parse_{{name}}(Node* p);
		{{/def}}
		{{#def}}
		{{#isInternal}}
		Node* Traverse_{{name}}(Node* p, PTNodeChildren& v);
		{{/isInternal}}
		{{/def}}
	};{{BI_NEWLINE}}
	
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
		PTItr GetNext(SymbolType _childT);{{BI_NEWLINE}}
		
	private:
		PTItr(boost::shared_ptr<Parser> _pParser, boost::shared_ptr<PTNodeChildren> _pSiblings, SymbolType _childType);
		PTItr(const PTItr& _iOther, SymbolType _childType);
		void GoToNext(SymbolType _childType);
		void SkipSiblingsWithWrongType(SymbolType _childType);
		boost::shared_ptr<PTNodeChildren> GetChildren();{{BI_NEWLINE}}
		
		SymbolType mType;
		Node* mpNode;
		boost::shared_ptr<PTNodeChildren> mpSiblings;
		PTNodeChildren::iterator miCurrent;
		boost::shared_ptr<PTNodeChildren> mpChildren;
		boost::shared_ptr<Parser> mpParser;
	};{{BI_NEWLINE}}

	std::ostream& operator<<(std::ostream& _os, const PTItr& _i);
}