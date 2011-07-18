#include <cassert>
#include <stdexcept>
#include <ostream>
#include <sstream>
#include <iomanip>
{{header}}

using namespace {{name}};{{BI_NEWLINE}}

namespace
{
	typedef SymbolTypeToPtr::value_type Memo;
	typedef SymbolTypeToPtr::iterator MemoIterator;
	typedef std::pair<SymbolTypeToPtr::iterator, bool> MemoInsertResult;{{BI_NEWLINE}}
	
	struct EscapeChar
	{
		EscapeChar(char _c) : c(_c) {}
		char c;
	};{{BI_NEWLINE}}
	
	std::ostream& operator<<(std::ostream& _os, EscapeChar _e)
	{
		char c = _e.c;
		switch (c)
		{
			case '\\': c = '\\'; break;
			case '\n': c = 'n';  break;
			case '\r': c = 'r';  break;
			case '\t': c = 't';  break;
			case '\'': c = '\''; break;
			case '\"': c = '\"'; break;
			
			default:
				_os.put(c);
				return _os;
		}
		
		_os.put('\\');
		_os.put(c);
		return _os;
	}{{BI_NEWLINE}}
	
	Node* Visit(SymbolType type, Node* p, PTNodeChildren& v)
	{
		MemoIterator i = p->end.find(type);
		assert(i != p->end.end());
		Node* pEnd = i->second;
		if (pEnd)
			v.push_back(PTNodeChild(type, p));
		return pEnd;
	}{{BI_NEWLINE}}
	
	struct Private
	{{{#def}}{{BI_NEWLINE}}		static Node* Parse_{{name}}(Node* p)
		{
			{{#isMemoized}}
			MemoInsertResult r = p->end.insert(Memo(SymbolType_{{name}}, 0));
			if (!r.second) return r.first->second;
			{{/isMemoized}}
			{{>parseCode}}
			{{#isMemoized}}
			r.first->second = p;
			{{/isMemoized}}
			return p;
		}
		{{/def}}
		
		{{#def}}
		{{#isInternal}}{{BI_NEWLINE}}		static Node* Traverse_{{name}}(Node* p, PTNodeChildren& v)
		{
			Node* e = Parse_{{name}}(p);
			if (!e)
				return 0;
			{{>traverseCode}}
			return e;
		}
		{{/isInternal}}
		{{/def}}
	};
}{{BI_NEWLINE}}

namespace {{name}}
{
	Node* Parse(SymbolType _type, Node* _symbol)
	{
		switch (_type)
		{
			{{#def}}
			case SymbolType_{{name}}: return Private::Parse_{{name}}(_symbol);
			{{/def}}
		}
		return 0;
	}{{BI_NEWLINE}}
	
	Node* Traverse(SymbolType _type, Node* _symbol, PTNodeChildren& _children)
	{
		switch (_type)
		{
			{{#def}}
			{{#isInternal}}
			case SymbolType_{{name}}: return Private::Traverse_{{name}}(_symbol, _children);
			{{/isInternal}}
			{{/def}}
			{{#def}}
			{{#isLeaf}}
			case SymbolType_{{name}}:
			{{/isLeaf}}
			{{/def}}
			default:
				return Parse(_type, _symbol);
		}
	}{{BI_NEWLINE}}
	
	const char* SymbolName(SymbolType _type)
	{
		switch (_type)
		{
			{{#def}}
			case SymbolType_{{name}}: return "{{name}}";
			{{/def}}
		}
	}{{BI_NEWLINE}}
	
	void Print(std::ostream& _os, SymbolType _type, Node* _pNode, int _tabs, int _maxLineSize)
	{
		PTNodeChildren children;
		Node* pEnd = Traverse(_type, _pNode, children);
		if (!pEnd)
		{
			std::ostringstream oss;
			oss << "Parsing Failed for \"" << SymbolName(_type) << "\"";
			throw std::runtime_error(oss.str());
		}{{BI_NEWLINE}}

		int tabCount = _tabs;
		while (tabCount--)
		  _os << "    ";{{BI_NEWLINE}}
		
		_os << SymbolName(_type) << ": \"";{{BI_NEWLINE}}

		size_t lineSize = 0;
		for (Node* p = _pNode; p != pEnd; ++p)
		{
			_os << EscapeChar(p->value);
			if (++lineSize >= _maxLineSize)
			{
				_os << "...";
				break;
			}
		}{{BI_NEWLINE}}
		
		_os << "\"\n";{{BI_NEWLINE}}
		
		for (PTNodeChildren::iterator i = children.begin(), iEnd = children.end(); i != iEnd; ++i)
			Print(_os, i->first, i->second, _tabs + 1, _maxLineSize);
	}
}
