#include "{{name}}.h"

namespace {{name}}
{
	namespace
	{
		PTNode* ParseRange(char _rangeBegin, char _rangeEnd, PTNode* _symbol)
		{
			if (_symbol->value < _rangeBegin)
				return 0;
			if (_symbol->value > _rangeEnd)
				return 0;
			return ++_symbol;
		}
		
		PTNode* ParseChar(char _char, PTNode* _symbol)
		{
			if (_symbol->value != _char)
				return 0;
			return ++_symbol;
		}
		
		PTNode* ParseAnyChar(PTNode* _symbol)
		{
			if (_symbol->value == 0)
				return 0;
			return ++_symbol;
		}
		
		PTNode* GetEnd(const PTNode* _symbol, PTNodeType _type)
		{
			PTNodeTypeToPtr::const_iterator i = _symbol->end.find(_type);
			if (i == _symbol->end.end())
				return 0;
			return i->second;
		}
		
		void SetEnd(PTNode* _symbol, PTNodeType _type, PTNode* _end)
		{
			_symbol->end[_type] = _end;
		}
		
		PTNode* Visit(PTNode* _symbol, PTNodeType _type, PTNodeVisitor& _visitor)
		{
			PTNode* end = GetEnd(_symbol, _type);
			if (end)
				_visitor(_symbol, _type);
			return end;
		}
		{{#def}}
		PTNode* Parse_{{name}}(PTNode*);{{/def}}
		{{#def}}
		PTNode* Parse_{{name}}(PTNode* p0)
		{
			{{parseCode}}
		}
{{/def}}{{#def}}
		PTNode* Traverse_{{name}}(PTNode*, PTNodeVisitor&);{{/def}}
		{{#def}}
		PTNode* Traverse_{{name}}(PTNode* p0, PTNodeVisitor& v)
		{
			{{traverseCode}}
		}
{{/def}}	}
	
	PTNode* Parse(PTNodeType _type, PTNode* _symbol)
	{
		switch (_type)
		{{{#def}}
			case PTNodeType_{{name}}: return Parse_{{name}}(_symbol);{{/def}}
		}
		return 0;
	}
	
	PTNode* Traverse(PTNodeType _type, PTNode* _symbol, PTNodeVisitor& _visitor)
	{
		switch (_type)
		{{{#def}}
			case PTNodeType_{{name}}: return Traverse_{{name}}(_symbol, _visitor);{{/def}}
		}
		return 0;
	}
}
