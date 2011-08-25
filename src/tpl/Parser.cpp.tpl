#ifndef BOOTSTRAP_TEST
#include <cassert>
#include <stdexcept>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <boost/format.hpp>
#endif{{BI_NEWLINE}}

{{header}}

namespace
{
	struct EscapeChar
	{
		EscapeChar(char _c);
		char c;
	};{{BI_NEWLINE}}
	
	EscapeChar::EscapeChar(char _c) : c(_c)
	{
	}{{BI_NEWLINE}}
	
	std::ostream& operator<<(std::ostream& _os, EscapeChar _e)
	{
		char c = _e.c;
		switch (c)
		{
			case '\n': c = 'n'; break;
			case '\r': c = 'r'; break;
			case '\t': c = 't'; break;{{BI_NEWLINE}}
			
			case '\\':
			case '\'':
			case '\"':
				break;{{BI_NEWLINE}}
				
			default:
				_os.put(c);
				return _os;
		}{{BI_NEWLINE}}
		
		_os.put('\\');
		_os.put(c);
		return _os;
	}
}{{BI_NEWLINE}}

const char* {{namespace}}::SymbolName({{namespace}}::SymbolType _type)
{
	switch (_type)
	{
		{{#def}}
		{{#isNode}}
		case SymbolType_{{name}}: return "{{name}}";
		{{/isNode}}
		{{/def}}
		default: throw std::runtime_error(str(boost::format("Invalid Symbol Type: %1%") % _type));
	}
}{{BI_NEWLINE}}

{{#def}}
{{#isNode}}
bool {{namespace}}::Parser::Parse_{{name}}(const char*& p)
{
	bool r = true;
	char c;
	{{>parseCode}}
	return r;
}{{BI_NEWLINE}}

{{/isNode}}
{{/def}}

bool {{namespace}}::Parser::Parse(SymbolType _type, const char*& p)
{
	const char* pBegin = p;
	if (fail[_type].count(pBegin))
		return false;
	EndMap::iterator i = end[_type].find(pBegin);
	if (i != end[_type].end())
	{
		p = i->second;
		return true;
	}{{BI_NEWLINE}}
	
	bool r = true;
	switch (_type)
	{
		{{#def}}
		{{#isNode}}
		case SymbolType_{{name}}: r = Parse_{{name}}(p); break;
		{{/isNode}}
		{{/def}}
		default:
			assert(false);
			return false;
	}{{BI_NEWLINE}}
	
	if (r)
		end[_type][pBegin] = p;
	else
		fail[_type].insert(pBegin);
	return r;
}{{BI_NEWLINE}}

bool {{namespace}}::Parser::Traverse({{namespace}}::SymbolType _type, const char*& p, {{namespace}}::Symbols& v)
{
	const char* pBegin = p;
	bool r = true;
	char c;
	switch (_type)
	{
		{{#def}}
		{{#isNode}}
		case SymbolType_{{name}}:
		{
			if (fail[SymbolType_{{name}}].count(pBegin))
				return false;
			{{>traverseCode}}
			if (r)
				end[SymbolType_{{name}}][pBegin] = p;
			else
				fail[SymbolType_{{name}}].insert(pBegin);
			return r;
		}{{BI_NEWLINE}}
		{{/isNode}}
		{{/def}}
			
		default:
			assert(false);
			return false;
	}
}{{BI_NEWLINE}}

void {{namespace}}::Parser::Print(std::ostream& _os, {{namespace}}::SymbolType _type, const char* _pNode, int _tabs, int _maxLineSize)
{
	Symbols children;
	const char* pEnd = _pNode;
	Traverse(_type, pEnd, children);
	//std::cout << children.size() << "\n";
	if (!pEnd)
		throw std::runtime_error(str(boost::format("Parsing Failed for \"%1%\"") % SymbolName(_type)));{{BI_NEWLINE}}

	int tabCount = _tabs;
	while (tabCount--)
		_os << "    ";{{BI_NEWLINE}}
	
	_os << SymbolName(_type) << ": \"";{{BI_NEWLINE}}

	size_t lineSize = 0;
	for (const char* p = _pNode; p != pEnd; ++p)
	{
		_os << EscapeChar(*p);
		if (++lineSize >= _maxLineSize)
		{
			_os << "...";
			break;
		}
	}{{BI_NEWLINE}}
	
	_os << "\"\n";{{BI_NEWLINE}}
	
	for (Symbols::iterator i = children.begin(), iEnd = children.end(); i != iEnd; ++i)
		Print(_os, i->type, i->value, _tabs + 1, _maxLineSize);

//*
	if (_tabs == 0)
	{
		_os << "Memo Count:\n";
		for (size_t k = 0; k < SymbolTypeCount; ++k)
			_os << boost::format("%1% %|20t|End: %2% %|40t|Fail: %3%\n") % SymbolName(SymbolType(k)) % end[k].size() % fail[k].size();
	}
//*/
}{{BI_NEWLINE}}

bool {{namespace}}::Parser::Visit({{namespace}}::SymbolType _type, const char*& _p, {{namespace}}::Symbols& _v)
{
	const char* pBegin = _p;
	bool r = Parse(_type, _p);
	if (r)
	{
		Symbol symbol = { _type, _p - pBegin, pBegin };
		_v.push_back(symbol);
	}
	return r;
}{{BI_NEWLINE}}

std::ostream& {{namespace}}::operator<<(std::ostream& _os, const {{namespace}}::Iterator& _i)
{
	if (_i)
		_os.write(_i->value, _i->length);
	return _os;
}{{BI_NEWLINE}}
