// This file automatically generated by template-converter:
//    /usr/local/bin/template-converter PEGParser.h.tpl.h /Users/alexk7/ipg/build/xcode/../../src/tpl/PEGParser.h.tpl /Users/alexk7/Library/Developer/Xcode/DerivedData/ipg-acuzlgobcqgvwyfrqqszjnxugixf/Build/Products/Debug/PEGParser.h.tpl.h
//
// DO NOT EDIT!

#ifndef PEGPARSER_H_TPL_H_
#define PEGPARSER_H_TPL_H_

#include <string>

const std::string PEGParser_h_tpl (
"#ifndef PEGPARSER_H\n"
"#define PEGPARSER_H\n"
"\n"
"#include <map>\n"
"\n"
"namespace {{name}}\n"
"{\n"
"	enum PTNodeType\n"
"	{\n"
"		{{#def}}PTNodeType_{{name}} = {{value}},\n"
"		{{/def}}\n"
"	};\n"
"\n"
"	struct PTNode;\n"
"	typedef std::map<PTNodeType, PTNode*> PTNodeTypeToPtr;\n"
"\n"
"	struct PTNode\n"
"	{\n"
"		char value;\n"
"		PTNodeTypeToPtr end;\n"
"	};\n"
"\n"
"	struct PTNodeVisitor\n"
"	{\n"
"		virtual void operator()(PTNode* _symbol, PTNodeType _type) = 0;\n"
"	};\n"
"\n"
"	PTNode* Parse(PTNodeType _type, PTNode* _symbol);\n"
"	PTNode* Traverse(PTNodeType _type, PTNode* _symbol, PTNodeVisitor& _visitor);\n"
"}\n"
"\n"
"#endif\n"
);

#endif /* PEGPARSER_H_TPL_H_ */
