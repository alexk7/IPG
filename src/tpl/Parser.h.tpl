// This file was automatically generated by IPG
// DO NOT EDIT!
#ifndef PEGPARSER_H
#define PEGPARSER_H

#include <map>

namespace {{name}}
{
	enum PTNodeType
	{
		{{#def}}PTNodeType_{{name}} = {{value}},
		{{/def}}
	};

	struct PTNode;
	typedef std::map<PTNodeType, PTNode*> PTNodeTypeToPtr;

	struct PTNode
	{
		char value;
		PTNodeTypeToPtr end;
	};

	struct PTNodeVisitor
	{
		virtual void operator()(PTNode* _symbol, PTNodeType _type) = 0;
	};

	PTNode* Parse(PTNodeType _type, PTNode* _symbol);
	PTNode* Traverse(PTNodeType _type, PTNode* _symbol, PTNodeVisitor& _visitor);
}

#endif
