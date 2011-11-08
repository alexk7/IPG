// This file was automatically generated by IPG on Tue Nov  8 10:32:33 2011
// (from /Users/alexk7/Documents/IPG/build/xcode/../../src/peg/peg.peg)
// DO NOT EDIT!
#ifndef BOOTSTRAP_TEST
#include <cassert>
#include <stdexcept>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <boost/format.hpp>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#endif

#ifndef BOOTSTRAP_TEST
#include <vector>
#include <boost/shared_ptr.hpp>
#endif

namespace PEGParser
{
	enum SymbolType
	{
		SymbolType_CLASS,
		SymbolType_Char,
		SymbolType_Definition,
		SymbolType_Expression,
		SymbolType_Grammar,
		SymbolType_Identifier,
		SymbolType_Item,
		SymbolType_LEFTARROW,
		SymbolType_LITERAL,
		SymbolType_Primary,
		SymbolType_Range,
		SymbolType_Sequence,
		SymbolTypeInvalid,
		SymbolTypeCount = SymbolTypeInvalid
	};

    typedef char Char;
    typedef const char* CharItr;
	struct Symbol
	{
		SymbolType type;
		size_t length;
		CharItr value;
        bool success;
	};

	CharItr SymbolName(SymbolType _type);

	class Context;

	class Iterator
	{
	public:
		friend Iterator Traverse(SymbolType _type, CharItr _text);
		friend Iterator Traverse(const Iterator& _iParent);
		friend void DebugPrint(std::ostream& _os, const Iterator& _i, int _tabs, int _maxLineSize);

		Iterator();
		Iterator& operator++();
		const Symbol& operator*() const;
		const Symbol* operator->() const;

	private:
		Iterator(boost::shared_ptr<Context> _pContext, boost::shared_ptr< std::vector<Symbol> > _pSiblings);

		boost::shared_ptr<Context> mpContext;
		boost::shared_ptr< std::vector<Symbol> > mpSiblings;
		std::vector<Symbol>::iterator mi;
	};

	Iterator Traverse(SymbolType _type, CharItr _text);
	Iterator Traverse(const Iterator& _iParent);
	void DebugPrint(std::ostream& _os, const Iterator& _i, int _tabs = 0, int _maxLineSize = 100);
}

using namespace std;
using namespace std::tr1;
using namespace boost;
using namespace PEGParser;

namespace
{
	struct Memo
	{
		unordered_map<CharItr, CharItr> end;
		unordered_set<CharItr> fail;
	};

	enum SkipType
	{
		SkipType_CLASS_1,
		SkipType_Expression_1,
		SkipType_Grammar_1,
		SkipType_IDENTIFIER_1,
		SkipType_Item_1,
		SkipType_Item_2,
		SkipType_LITERAL_1,
		SkipType_Primary_1,
		SkipType_Primary_2,
		SkipType_Primary_3,
		SkipTypeInvalid,
		SkipTypeCount = SkipTypeInvalid
	};

	struct EscapeChar
	{
		EscapeChar(Char _c);
		Char c;
	};

	EscapeChar::EscapeChar(Char _c) : c(_c)
	{
	}

	ostream& operator<<(ostream& _os, EscapeChar _e)
	{
		Char c = _e.c;
		switch (c)
		{
			case '\n': c = 'n'; break;
			case '\r': c = 'r'; break;
			case '\t': c = 't'; break;

			case '\\':
			case '\'':
			case '\"':
				break;

			default:
				_os.put(c);
				return _os;
		}

		_os.put('\\');
		_os.put(c);
		return _os;
	}

	bool ParseSymbol(Context&, SymbolType, CharItr&);

	typedef bool ParseFn(Context&, CharItr&);
	ParseFn Parse_AND, Parse_CLASS, Parse_CLASS_1, Parse_CLOSE, Parse_Char, Parse_Comment, Parse_DOT, Parse_Definition, Parse_EndOfFile, Parse_EndOfLine, Parse_Expression, Parse_Expression_1, Parse_Grammar, Parse_Grammar_1, Parse_IDENTIFIER, Parse_IDENTIFIER_1, Parse_Identifier, Parse_Item, Parse_Item_1, Parse_Item_2, Parse_LEFTARROW, Parse_LITERAL, Parse_LITERAL_1, Parse_NOT, Parse_OPEN, Parse_PLUS, Parse_Primary, Parse_Primary_1, Parse_Primary_2, Parse_Primary_3, Parse_QUESTION, Parse_Range, Parse_SLASH, Parse_STAR, Parse_Sequence, Parse_Space, Parse_Spacing;

	typedef bool TraverseFn(Context&, CharItr&, vector<Symbol>&);
	TraverseFn Traverse_AND, Traverse_CLASS, Traverse_CLASS_1, Traverse_CLOSE, Traverse_Char, Traverse_Comment, Traverse_DOT, Traverse_Definition, Traverse_EndOfFile, Traverse_EndOfLine, Traverse_Expression, Traverse_Expression_1, Traverse_Grammar, Traverse_Grammar_1, Traverse_IDENTIFIER, Traverse_IDENTIFIER_1, Traverse_Identifier, Traverse_Item, Traverse_Item_1, Traverse_Item_2, Traverse_LEFTARROW, Traverse_LITERAL, Traverse_LITERAL_1, Traverse_NOT, Traverse_OPEN, Traverse_PLUS, Traverse_Primary, Traverse_Primary_1, Traverse_Primary_2, Traverse_Primary_3, Traverse_QUESTION, Traverse_Range, Traverse_SLASH, Traverse_STAR, Traverse_Sequence, Traverse_Space, Traverse_Spacing;
}

CharItr PEGParser::SymbolName(SymbolType _type)
{
	switch (_type)
	{
		case SymbolType_CLASS: return "CLASS";
		case SymbolType_Char: return "Char";
		case SymbolType_Definition: return "Definition";
		case SymbolType_Expression: return "Expression";
		case SymbolType_Grammar: return "Grammar";
		case SymbolType_Identifier: return "Identifier";
		case SymbolType_Item: return "Item";
		case SymbolType_LEFTARROW: return "LEFTARROW";
		case SymbolType_LITERAL: return "LITERAL";
		case SymbolType_Primary: return "Primary";
		case SymbolType_Range: return "Range";
		case SymbolType_Sequence: return "Sequence";
		default: throw runtime_error(str(format("Invalid Symbol Type: %1%") % _type));
	}
}

class PEGParser::Context
{
public:
	Memo symbolMemos[SymbolTypeCount];
	Memo skipMemos[SkipTypeCount];
};

namespace
{
	bool Visit(Context& _ctx, SymbolType _type, CharItr& _p, vector<Symbol>& _v)
	{
		CharItr pBegin = _p;
		bool success = ParseSymbol(_ctx, _type, _p);
		if (!success) --_p;
        Symbol symbol = { _type, _p - pBegin, pBegin, success };
        _v.push_back(symbol);
		return success;
	}

	bool ParseMemoized(Context& _ctx, Memo& _memo, ParseFn* _parseFn, CharItr& p)
	{
		if (_memo.fail.count(p))
			return false;

		unordered_map<CharItr, CharItr>::iterator i = _memo.end.find(p);
		if (i != _memo.end.end())
		{
			p = i->second;
			return true;
		}

		CharItr pBegin = p;
		if (_parseFn(_ctx, p))
		{
			_memo.end[pBegin] = p;
			return true;
		}

		_memo.fail.insert(pBegin);
		return false;
	}

	bool ParseSymbol(Context& _ctx, SymbolType _type, CharItr& p)
	{
		switch (_type)
		{
			case SymbolType_CLASS: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_CLASS, p);
			case SymbolType_Char: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_Char, p);
			case SymbolType_Definition: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_Definition, p);
			case SymbolType_Expression: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_Expression, p);
			case SymbolType_Grammar: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_Grammar, p);
			case SymbolType_Identifier: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_Identifier, p);
			case SymbolType_Item: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_Item, p);
			case SymbolType_LEFTARROW: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_LEFTARROW, p);
			case SymbolType_LITERAL: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_LITERAL, p);
			case SymbolType_Primary: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_Primary, p);
			case SymbolType_Range: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_Range, p);
			case SymbolType_Sequence: return ParseMemoized(_ctx, _ctx.symbolMemos[_type], Parse_Sequence, p);
			default:
				assert(false);
				return false;
		}
	}

	bool ParseSkip(Context& _ctx, SkipType _type, CharItr& p)
	{
		switch (_type)
		{
			case SkipType_CLASS_1: return ParseMemoized(_ctx, _ctx.skipMemos[_type], Parse_CLASS_1, p);
			case SkipType_Expression_1: return ParseMemoized(_ctx, _ctx.skipMemos[_type], Parse_Expression_1, p);
			case SkipType_Grammar_1: return ParseMemoized(_ctx, _ctx.skipMemos[_type], Parse_Grammar_1, p);
			case SkipType_IDENTIFIER_1: return ParseMemoized(_ctx, _ctx.skipMemos[_type], Parse_IDENTIFIER_1, p);
			case SkipType_Item_1: return ParseMemoized(_ctx, _ctx.skipMemos[_type], Parse_Item_1, p);
			case SkipType_Item_2: return ParseMemoized(_ctx, _ctx.skipMemos[_type], Parse_Item_2, p);
			case SkipType_LITERAL_1: return ParseMemoized(_ctx, _ctx.skipMemos[_type], Parse_LITERAL_1, p);
			case SkipType_Primary_1: return ParseMemoized(_ctx, _ctx.skipMemos[_type], Parse_Primary_1, p);
			case SkipType_Primary_2: return ParseMemoized(_ctx, _ctx.skipMemos[_type], Parse_Primary_2, p);
			case SkipType_Primary_3: return ParseMemoized(_ctx, _ctx.skipMemos[_type], Parse_Primary_3, p);
			default:
				assert(false);
				return false;
		}
	}

	bool Parse_AND(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '&');
		if (r)
		{
			r = Parse_Spacing(_ctx, p);
		}
		return r;
	}

	bool Parse_CLASS(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '[');
		if (r)
		{
			for (;;)
			{
				const char* b = p;
				char c1 = *p++;
				r = (c1 == ']');
				r = !r;
				p = b;
				if (r)
				{
					r = ParseSymbol(_ctx, SymbolType_Range, p);
				}
				if (!r)
				{
					p = b;
					break;
				}
			}
			r = true;
			if (r)
			{
				r = ParseSkip(_ctx, SkipType_CLASS_1, p);
			}
		}
		return r;
	}

	bool Parse_CLASS_1(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == ']');
		if (r)
		{
			r = Parse_Spacing(_ctx, p);
		}
		return r;
	}

	bool Parse_CLOSE(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == ')');
		if (r)
		{
			r = Parse_Spacing(_ctx, p);
		}
		return r;
	}

	bool Parse_Char(Context& _ctx, CharItr& p)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 == '\\');
		if (r)
		{
			const char* b2 = p;
			char c1 = *p++;
			r = (c1 == 'n');
			if (!r)
			{
				p = b2;
				char c2 = *p++;
				r = (c2 == 'r');
				if (!r)
				{
					p = b2;
					char c3 = *p++;
					r = (c3 == 't');
					if (!r)
					{
						p = b2;
						char c4 = *p++;
						r = (c4 == '\'');
						if (!r)
						{
							p = b2;
							char c5 = *p++;
							r = (c5 == '\"');
							if (!r)
							{
								p = b2;
								char c6 = *p++;
								r = (c6 == '[');
								if (!r)
								{
									p = b2;
									char c7 = *p++;
									r = (c7 == ']');
									if (!r)
									{
										p = b2;
										char c8 = *p++;
										r = (c8 == '\\');
										if (!r)
										{
											p = b2;
											char c9 = *p++;
											r = (c9 >= '1' && c9 <= '9');
											if (r)
											{
												for (;;)
												{
													const char* b3 = p;
													char c10 = *p++;
													r = (c10 >= '0' && c10 <= '9');
													if (!r)
													{
														p = b3;
														break;
													}
												}
												r = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		if (!r)
		{
			p = b;
			char c11 = *p++;
			r = (c11 == '\\');
			r = !r;
			p = b;
			if (r)
			{
				char c12 = *p++;
				r = (c12 != 0);
			}
		}
		return r;
	}

	bool Parse_Comment(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '#');
		if (r)
		{
			for (;;)
			{
				const char* b = p;
				r = Parse_EndOfLine(_ctx, p);
				r = !r;
				p = b;
				if (r)
				{
					char c1 = *p++;
					r = (c1 != 0);
				}
				if (!r)
				{
					p = b;
					break;
				}
			}
			r = true;
			if (r)
			{
				r = Parse_EndOfLine(_ctx, p);
			}
		}
		return r;
	}

	bool Parse_DOT(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '.');
		if (r)
		{
			r = Parse_Spacing(_ctx, p);
		}
		return r;
	}

	bool Parse_Definition(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = Parse_IDENTIFIER(_ctx, p);
		if (r)
		{
			r = ParseSymbol(_ctx, SymbolType_LEFTARROW, p);
			if (r)
			{
				r = ParseSymbol(_ctx, SymbolType_Expression, p);
			}
		}
		return r;
	}

	bool Parse_EndOfFile(Context& _ctx, CharItr& p)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 != 0);
		r = !r;
		p = b;
		return r;
	}

	bool Parse_EndOfLine(Context& _ctx, CharItr& p)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 == '\n');
		if (!r)
		{
			p = b;
			char c1 = *p++;
			r = (c1 == '\r');
			if (r)
			{
				const char* b2 = p;
				char c2 = *p++;
				r = (c2 == '\n');
				if (!r)
				{
					p = b2;
					r = true;
				}
			}
		}
		return r;
	}

	bool Parse_Expression(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = ParseSymbol(_ctx, SymbolType_Sequence, p);
		if (r)
		{
			for (;;)
			{
				const char* b = p;
				r = ParseSkip(_ctx, SkipType_Expression_1, p);
				if (r)
				{
					r = ParseSymbol(_ctx, SymbolType_Sequence, p);
				}
				if (!r)
				{
					p = b;
					break;
				}
			}
			r = true;
		}
		return r;
	}

	bool Parse_Expression_1(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = Parse_SLASH(_ctx, p);
		return r;
	}

	bool Parse_Grammar(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = ParseSkip(_ctx, SkipType_Grammar_1, p);
		if (r)
		{
			r = ParseSymbol(_ctx, SymbolType_Definition, p);
			if (r)
			{
				for (;;)
				{
					const char* b = p;
					r = ParseSymbol(_ctx, SymbolType_Definition, p);
					if (!r)
					{
						p = b;
						break;
					}
				}
				r = true;
				if (r)
				{
					r = Parse_EndOfFile(_ctx, p);
				}
			}
		}
		return r;
	}

	bool Parse_Grammar_1(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = Parse_Spacing(_ctx, p);
		return r;
	}

	bool Parse_IDENTIFIER(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = ParseSymbol(_ctx, SymbolType_Identifier, p);
		if (r)
		{
			r = ParseSkip(_ctx, SkipType_IDENTIFIER_1, p);
		}
		return r;
	}

	bool Parse_IDENTIFIER_1(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = Parse_Spacing(_ctx, p);
		return r;
	}

	bool Parse_Identifier(Context& _ctx, CharItr& p)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 >= 'a' && c0 <= 'z');
		if (!r)
		{
			p = b;
			char c1 = *p++;
			r = (c1 >= 'A' && c1 <= 'Z');
			if (!r)
			{
				p = b;
				char c2 = *p++;
				r = (c2 == '_');
			}
		}
		if (r)
		{
			for (;;)
			{
				const char* b2 = p;
				char c3 = *p++;
				r = (c3 >= 'a' && c3 <= 'z');
				if (!r)
				{
					p = b2;
					char c4 = *p++;
					r = (c4 >= 'A' && c4 <= 'Z');
					if (!r)
					{
						p = b2;
						char c5 = *p++;
						r = (c5 >= '0' && c5 <= '9');
						if (!r)
						{
							p = b2;
							char c6 = *p++;
							r = (c6 == '_');
						}
					}
				}
				if (!r)
				{
					p = b2;
					break;
				}
			}
			r = true;
		}
		return r;
	}

	bool Parse_Item(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = ParseSkip(_ctx, SkipType_Item_1, p);
		if (r)
		{
			r = ParseSymbol(_ctx, SymbolType_Primary, p);
			if (r)
			{
				r = ParseSkip(_ctx, SkipType_Item_2, p);
			}
		}
		return r;
	}

	bool Parse_Item_1(Context& _ctx, CharItr& p)
	{
		bool r = true;
		const char* b = p;
		r = Parse_AND(_ctx, p);
		if (!r)
		{
			p = b;
			r = Parse_NOT(_ctx, p);
			if (!r)
			{
				p = b;
				r = true;
			}
		}
		return r;
	}

	bool Parse_Item_2(Context& _ctx, CharItr& p)
	{
		bool r = true;
		const char* b = p;
		r = Parse_QUESTION(_ctx, p);
		if (!r)
		{
			p = b;
			r = Parse_STAR(_ctx, p);
			if (!r)
			{
				p = b;
				r = Parse_PLUS(_ctx, p);
				if (!r)
				{
					p = b;
					r = true;
				}
			}
		}
		return r;
	}

	bool Parse_LEFTARROW(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '<');
		if (r)
		{
			const char* b = p;
			char c1 = *p++;
			r = (c1 == '-');
			if (!r)
			{
				p = b;
				char c2 = *p++;
				r = (c2 == '=');
			}
			if (r)
			{
				r = Parse_Spacing(_ctx, p);
			}
		}
		return r;
	}

	bool Parse_LITERAL(Context& _ctx, CharItr& p)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 == '\'');
		if (r)
		{
			for (;;)
			{
				const char* b2 = p;
				char c1 = *p++;
				r = (c1 == '\'');
				r = !r;
				p = b2;
				if (r)
				{
					r = ParseSymbol(_ctx, SymbolType_Char, p);
				}
				if (!r)
				{
					p = b2;
					break;
				}
			}
			r = true;
			if (r)
			{
				char c2 = *p++;
				r = (c2 == '\'');
			}
		}
		if (!r)
		{
			p = b;
			char c3 = *p++;
			r = (c3 == '\"');
			if (r)
			{
				for (;;)
				{
					const char* b3 = p;
					char c4 = *p++;
					r = (c4 == '\"');
					r = !r;
					p = b3;
					if (r)
					{
						r = ParseSymbol(_ctx, SymbolType_Char, p);
					}
					if (!r)
					{
						p = b3;
						break;
					}
				}
				r = true;
				if (r)
				{
					char c5 = *p++;
					r = (c5 == '\"');
				}
			}
		}
		if (r)
		{
			r = ParseSkip(_ctx, SkipType_LITERAL_1, p);
		}
		return r;
	}

	bool Parse_LITERAL_1(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = Parse_Spacing(_ctx, p);
		return r;
	}

	bool Parse_NOT(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '!');
		if (r)
		{
			r = Parse_Spacing(_ctx, p);
		}
		return r;
	}

	bool Parse_OPEN(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '(');
		if (r)
		{
			r = Parse_Spacing(_ctx, p);
		}
		return r;
	}

	bool Parse_PLUS(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '+');
		if (r)
		{
			r = Parse_Spacing(_ctx, p);
		}
		return r;
	}

	bool Parse_Primary(Context& _ctx, CharItr& p)
	{
		bool r = true;
		const char* b = p;
		r = Parse_IDENTIFIER(_ctx, p);
		if (r)
		{
			const char* b2 = p;
			r = ParseSymbol(_ctx, SymbolType_LEFTARROW, p);
			r = !r;
			p = b2;
		}
		if (!r)
		{
			p = b;
			r = ParseSkip(_ctx, SkipType_Primary_1, p);
			if (r)
			{
				r = ParseSymbol(_ctx, SymbolType_Expression, p);
				if (r)
				{
					r = ParseSkip(_ctx, SkipType_Primary_2, p);
				}
			}
			if (!r)
			{
				p = b;
				r = ParseSymbol(_ctx, SymbolType_LITERAL, p);
				if (!r)
				{
					p = b;
					r = ParseSymbol(_ctx, SymbolType_CLASS, p);
					if (!r)
					{
						p = b;
						r = ParseSkip(_ctx, SkipType_Primary_3, p);
					}
				}
			}
		}
		return r;
	}

	bool Parse_Primary_1(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = Parse_OPEN(_ctx, p);
		return r;
	}

	bool Parse_Primary_2(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = Parse_CLOSE(_ctx, p);
		return r;
	}

	bool Parse_Primary_3(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = Parse_DOT(_ctx, p);
		return r;
	}

	bool Parse_QUESTION(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '?');
		if (r)
		{
			r = Parse_Spacing(_ctx, p);
		}
		return r;
	}

	bool Parse_Range(Context& _ctx, CharItr& p)
	{
		bool r = true;
		r = ParseSymbol(_ctx, SymbolType_Char, p);
		if (r)
		{
			const char* b = p;
			char c0 = *p++;
			r = (c0 == '-');
			if (r)
			{
				r = ParseSymbol(_ctx, SymbolType_Char, p);
			}
			if (!r)
			{
				p = b;
				r = true;
			}
		}
		return r;
	}

	bool Parse_SLASH(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '/');
		if (r)
		{
			r = Parse_Spacing(_ctx, p);
		}
		return r;
	}

	bool Parse_STAR(Context& _ctx, CharItr& p)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '*');
		if (r)
		{
			r = Parse_Spacing(_ctx, p);
		}
		return r;
	}

	bool Parse_Sequence(Context& _ctx, CharItr& p)
	{
		bool r = true;
		for (;;)
		{
			const char* b = p;
			r = ParseSymbol(_ctx, SymbolType_Item, p);
			if (!r)
			{
				p = b;
				break;
			}
		}
		r = true;
		return r;
	}

	bool Parse_Space(Context& _ctx, CharItr& p)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 == ' ');
		if (!r)
		{
			p = b;
			char c1 = *p++;
			r = (c1 == '\t');
			if (!r)
			{
				p = b;
				r = Parse_EndOfLine(_ctx, p);
			}
		}
		return r;
	}

	bool Parse_Spacing(Context& _ctx, CharItr& p)
	{
		bool r = true;
		for (;;)
		{
			const char* b = p;
			r = Parse_Space(_ctx, p);
			if (!r)
			{
				p = b;
				r = Parse_Comment(_ctx, p);
			}
			if (!r)
			{
				p = b;
				break;
			}
		}
		r = true;
		return r;
	}

	bool TraverseMemoized(Context& _ctx, Memo& _memo, TraverseFn* _traverseFn, CharItr& p, vector<Symbol>& v)
	{
		if (_memo.fail.count(p))
			return false;

		CharItr pBegin = p;
		if (_traverseFn(_ctx, p, v))
		{
			_memo.end[pBegin] = p;
			return true;
		}

		_memo.fail.insert(pBegin);
		return false;		
	}

	bool TraverseSymbol(Context& _ctx, SymbolType _type, CharItr& p, vector<Symbol>& v)
	{
		switch (_type)
		{
			case SymbolType_CLASS: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_CLASS, p, v);
			case SymbolType_Char: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_Char, p, v);
			case SymbolType_Definition: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_Definition, p, v);
			case SymbolType_Expression: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_Expression, p, v);
			case SymbolType_Grammar: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_Grammar, p, v);
			case SymbolType_Identifier: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_Identifier, p, v);
			case SymbolType_Item: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_Item, p, v);
			case SymbolType_LEFTARROW: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_LEFTARROW, p, v);
			case SymbolType_LITERAL: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_LITERAL, p, v);
			case SymbolType_Primary: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_Primary, p, v);
			case SymbolType_Range: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_Range, p, v);
			case SymbolType_Sequence: return TraverseMemoized(_ctx, _ctx.symbolMemos[_type], Traverse_Sequence, p, v);
			default:
				assert(false);
				return false;
		}
	}

	bool TraverseSkip(Context& _ctx, SkipType _type, CharItr& p, vector<Symbol>& v)
	{
		switch (_type)
		{
			case SkipType_CLASS_1: return TraverseMemoized(_ctx, _ctx.skipMemos[_type], Traverse_CLASS_1, p, v);
			case SkipType_Expression_1: return TraverseMemoized(_ctx, _ctx.skipMemos[_type], Traverse_Expression_1, p, v);
			case SkipType_Grammar_1: return TraverseMemoized(_ctx, _ctx.skipMemos[_type], Traverse_Grammar_1, p, v);
			case SkipType_IDENTIFIER_1: return TraverseMemoized(_ctx, _ctx.skipMemos[_type], Traverse_IDENTIFIER_1, p, v);
			case SkipType_Item_1: return TraverseMemoized(_ctx, _ctx.skipMemos[_type], Traverse_Item_1, p, v);
			case SkipType_Item_2: return TraverseMemoized(_ctx, _ctx.skipMemos[_type], Traverse_Item_2, p, v);
			case SkipType_LITERAL_1: return TraverseMemoized(_ctx, _ctx.skipMemos[_type], Traverse_LITERAL_1, p, v);
			case SkipType_Primary_1: return TraverseMemoized(_ctx, _ctx.skipMemos[_type], Traverse_Primary_1, p, v);
			case SkipType_Primary_2: return TraverseMemoized(_ctx, _ctx.skipMemos[_type], Traverse_Primary_2, p, v);
			case SkipType_Primary_3: return TraverseMemoized(_ctx, _ctx.skipMemos[_type], Traverse_Primary_3, p, v);
			default:
				assert(false);
				return false;
		}
	}

	bool Traverse_AND(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '&');
		if (r)
		{
			r = Traverse_Spacing(_ctx, p, v);
		}
		return r;
	}

	bool Traverse_CLASS(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '[');
		if (r)
		{
			for (;;)
			{
				const char* b = p;
				size_t s = v.size();
				char c1 = *p++;
				r = (c1 == ']');
				r = !r;
				p = b;
				if (r)
				{
					r = Visit(_ctx, SymbolType_Range, p, v);
				}
				if (!r)
				{
					p = b;
					v.erase(v.begin() + s, v.end());
					break;
				}
			}
			r = true;
			if (r)
			{
				r = TraverseSkip(_ctx, SkipType_CLASS_1, p, v);
			}
		}
		return r;
	}

	bool Traverse_CLASS_1(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == ']');
		if (r)
		{
			r = Traverse_Spacing(_ctx, p, v);
		}
		return r;
	}

	bool Traverse_CLOSE(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == ')');
		if (r)
		{
			r = Traverse_Spacing(_ctx, p, v);
		}
		return r;
	}

	bool Traverse_Char(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 == '\\');
		if (r)
		{
			const char* b2 = p;
			char c1 = *p++;
			r = (c1 == 'n');
			if (!r)
			{
				p = b2;
				char c2 = *p++;
				r = (c2 == 'r');
				if (!r)
				{
					p = b2;
					char c3 = *p++;
					r = (c3 == 't');
					if (!r)
					{
						p = b2;
						char c4 = *p++;
						r = (c4 == '\'');
						if (!r)
						{
							p = b2;
							char c5 = *p++;
							r = (c5 == '\"');
							if (!r)
							{
								p = b2;
								char c6 = *p++;
								r = (c6 == '[');
								if (!r)
								{
									p = b2;
									char c7 = *p++;
									r = (c7 == ']');
									if (!r)
									{
										p = b2;
										char c8 = *p++;
										r = (c8 == '\\');
										if (!r)
										{
											p = b2;
											char c9 = *p++;
											r = (c9 >= '1' && c9 <= '9');
											if (r)
											{
												for (;;)
												{
													const char* b3 = p;
													char c10 = *p++;
													r = (c10 >= '0' && c10 <= '9');
													if (!r)
													{
														p = b3;
														break;
													}
												}
												r = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		if (!r)
		{
			p = b;
			char c11 = *p++;
			r = (c11 == '\\');
			r = !r;
			p = b;
			if (r)
			{
				char c12 = *p++;
				r = (c12 != 0);
			}
		}
		return r;
	}

	bool Traverse_Comment(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '#');
		if (r)
		{
			for (;;)
			{
				const char* b = p;
				r = Parse_EndOfLine(_ctx, p);
				r = !r;
				p = b;
				if (r)
				{
					char c1 = *p++;
					r = (c1 != 0);
				}
				if (!r)
				{
					p = b;
					break;
				}
			}
			r = true;
			if (r)
			{
				r = Traverse_EndOfLine(_ctx, p, v);
			}
		}
		return r;
	}

	bool Traverse_DOT(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '.');
		if (r)
		{
			r = Traverse_Spacing(_ctx, p, v);
		}
		return r;
	}

	bool Traverse_Definition(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Traverse_IDENTIFIER(_ctx, p, v);
		if (r)
		{
			r = Visit(_ctx, SymbolType_LEFTARROW, p, v);
			if (r)
			{
				r = Visit(_ctx, SymbolType_Expression, p, v);
			}
		}
		return r;
	}

	bool Traverse_EndOfFile(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 != 0);
		r = !r;
		p = b;
		return r;
	}

	bool Traverse_EndOfLine(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 == '\n');
		if (!r)
		{
			p = b;
			char c1 = *p++;
			r = (c1 == '\r');
			if (r)
			{
				const char* b2 = p;
				char c2 = *p++;
				r = (c2 == '\n');
				if (!r)
				{
					p = b2;
					r = true;
				}
			}
		}
		return r;
	}

	bool Traverse_Expression(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Visit(_ctx, SymbolType_Sequence, p, v);
		if (r)
		{
			for (;;)
			{
				const char* b = p;
				size_t s = v.size();
				r = TraverseSkip(_ctx, SkipType_Expression_1, p, v);
				if (r)
				{
					r = Visit(_ctx, SymbolType_Sequence, p, v);
				}
				if (!r)
				{
					p = b;
					v.erase(v.begin() + s, v.end());
					break;
				}
			}
			r = true;
		}
		return r;
	}

	bool Traverse_Expression_1(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Traverse_SLASH(_ctx, p, v);
		return r;
	}

	bool Traverse_Grammar(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = TraverseSkip(_ctx, SkipType_Grammar_1, p, v);
		if (r)
		{
			r = Visit(_ctx, SymbolType_Definition, p, v);
			if (r)
			{
				for (;;)
				{
					const char* b = p;
					size_t s = v.size();
					r = Visit(_ctx, SymbolType_Definition, p, v);
					if (!r)
					{
						p = b;
						v.erase(v.begin() + s, v.end());
						break;
					}
				}
				r = true;
				if (r)
				{
					r = Traverse_EndOfFile(_ctx, p, v);
				}
			}
		}
		return r;
	}

	bool Traverse_Grammar_1(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Traverse_Spacing(_ctx, p, v);
		return r;
	}

	bool Traverse_IDENTIFIER(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Visit(_ctx, SymbolType_Identifier, p, v);
		if (r)
		{
			r = TraverseSkip(_ctx, SkipType_IDENTIFIER_1, p, v);
		}
		return r;
	}

	bool Traverse_IDENTIFIER_1(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Traverse_Spacing(_ctx, p, v);
		return r;
	}

	bool Traverse_Identifier(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 >= 'a' && c0 <= 'z');
		if (!r)
		{
			p = b;
			char c1 = *p++;
			r = (c1 >= 'A' && c1 <= 'Z');
			if (!r)
			{
				p = b;
				char c2 = *p++;
				r = (c2 == '_');
			}
		}
		if (r)
		{
			for (;;)
			{
				const char* b2 = p;
				char c3 = *p++;
				r = (c3 >= 'a' && c3 <= 'z');
				if (!r)
				{
					p = b2;
					char c4 = *p++;
					r = (c4 >= 'A' && c4 <= 'Z');
					if (!r)
					{
						p = b2;
						char c5 = *p++;
						r = (c5 >= '0' && c5 <= '9');
						if (!r)
						{
							p = b2;
							char c6 = *p++;
							r = (c6 == '_');
						}
					}
				}
				if (!r)
				{
					p = b2;
					break;
				}
			}
			r = true;
		}
		return r;
	}

	bool Traverse_Item(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = TraverseSkip(_ctx, SkipType_Item_1, p, v);
		if (r)
		{
			r = Visit(_ctx, SymbolType_Primary, p, v);
			if (r)
			{
				r = TraverseSkip(_ctx, SkipType_Item_2, p, v);
			}
		}
		return r;
	}

	bool Traverse_Item_1(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		const char* b = p;
		r = Traverse_AND(_ctx, p, v);
		if (!r)
		{
			p = b;
			r = Traverse_NOT(_ctx, p, v);
			if (!r)
			{
				p = b;
				r = true;
			}
		}
		return r;
	}

	bool Traverse_Item_2(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		const char* b = p;
		r = Traverse_QUESTION(_ctx, p, v);
		if (!r)
		{
			p = b;
			r = Traverse_STAR(_ctx, p, v);
			if (!r)
			{
				p = b;
				r = Traverse_PLUS(_ctx, p, v);
				if (!r)
				{
					p = b;
					r = true;
				}
			}
		}
		return r;
	}

	bool Traverse_LEFTARROW(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '<');
		if (r)
		{
			const char* b = p;
			char c1 = *p++;
			r = (c1 == '-');
			if (!r)
			{
				p = b;
				char c2 = *p++;
				r = (c2 == '=');
			}
			if (r)
			{
				r = Traverse_Spacing(_ctx, p, v);
			}
		}
		return r;
	}

	bool Traverse_LITERAL(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		const char* b = p;
		size_t s = v.size();
		char c0 = *p++;
		r = (c0 == '\'');
		if (r)
		{
			for (;;)
			{
				const char* b2 = p;
				size_t s2 = v.size();
				char c1 = *p++;
				r = (c1 == '\'');
				r = !r;
				p = b2;
				if (r)
				{
					r = Visit(_ctx, SymbolType_Char, p, v);
				}
				if (!r)
				{
					p = b2;
					v.erase(v.begin() + s2, v.end());
					break;
				}
			}
			r = true;
			if (r)
			{
				char c2 = *p++;
				r = (c2 == '\'');
			}
		}
		if (!r)
		{
			p = b;
			v.erase(v.begin() + s, v.end());
			char c3 = *p++;
			r = (c3 == '\"');
			if (r)
			{
				for (;;)
				{
					const char* b3 = p;
					size_t s3 = v.size();
					char c4 = *p++;
					r = (c4 == '\"');
					r = !r;
					p = b3;
					if (r)
					{
						r = Visit(_ctx, SymbolType_Char, p, v);
					}
					if (!r)
					{
						p = b3;
						v.erase(v.begin() + s3, v.end());
						break;
					}
				}
				r = true;
				if (r)
				{
					char c5 = *p++;
					r = (c5 == '\"');
				}
			}
		}
		if (r)
		{
			r = TraverseSkip(_ctx, SkipType_LITERAL_1, p, v);
		}
		return r;
	}

	bool Traverse_LITERAL_1(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Traverse_Spacing(_ctx, p, v);
		return r;
	}

	bool Traverse_NOT(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '!');
		if (r)
		{
			r = Traverse_Spacing(_ctx, p, v);
		}
		return r;
	}

	bool Traverse_OPEN(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '(');
		if (r)
		{
			r = Traverse_Spacing(_ctx, p, v);
		}
		return r;
	}

	bool Traverse_PLUS(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '+');
		if (r)
		{
			r = Traverse_Spacing(_ctx, p, v);
		}
		return r;
	}

	bool Traverse_Primary(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		const char* b = p;
		size_t s = v.size();
		r = Traverse_IDENTIFIER(_ctx, p, v);
		if (r)
		{
			const char* b2 = p;
			r = ParseSymbol(_ctx, SymbolType_LEFTARROW, p);
			r = !r;
			p = b2;
		}
		if (!r)
		{
			p = b;
			v.erase(v.begin() + s, v.end());
			r = TraverseSkip(_ctx, SkipType_Primary_1, p, v);
			if (r)
			{
				r = Visit(_ctx, SymbolType_Expression, p, v);
				if (r)
				{
					r = TraverseSkip(_ctx, SkipType_Primary_2, p, v);
				}
			}
			if (!r)
			{
				p = b;
				v.erase(v.begin() + s, v.end());
				r = Visit(_ctx, SymbolType_LITERAL, p, v);
				if (!r)
				{
					p = b;
					v.erase(v.begin() + s, v.end());
					r = Visit(_ctx, SymbolType_CLASS, p, v);
					if (!r)
					{
						p = b;
						v.erase(v.begin() + s, v.end());
						r = TraverseSkip(_ctx, SkipType_Primary_3, p, v);
					}
				}
			}
		}
		return r;
	}

	bool Traverse_Primary_1(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Traverse_OPEN(_ctx, p, v);
		return r;
	}

	bool Traverse_Primary_2(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Traverse_CLOSE(_ctx, p, v);
		return r;
	}

	bool Traverse_Primary_3(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Traverse_DOT(_ctx, p, v);
		return r;
	}

	bool Traverse_QUESTION(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '?');
		if (r)
		{
			r = Traverse_Spacing(_ctx, p, v);
		}
		return r;
	}

	bool Traverse_Range(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		r = Visit(_ctx, SymbolType_Char, p, v);
		if (r)
		{
			const char* b = p;
			size_t s = v.size();
			char c0 = *p++;
			r = (c0 == '-');
			if (r)
			{
				r = Visit(_ctx, SymbolType_Char, p, v);
			}
			if (!r)
			{
				p = b;
				v.erase(v.begin() + s, v.end());
				r = true;
			}
		}
		return r;
	}

	bool Traverse_SLASH(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '/');
		if (r)
		{
			r = Traverse_Spacing(_ctx, p, v);
		}
		return r;
	}

	bool Traverse_STAR(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		char c0 = *p++;
		r = (c0 == '*');
		if (r)
		{
			r = Traverse_Spacing(_ctx, p, v);
		}
		return r;
	}

	bool Traverse_Sequence(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		for (;;)
		{
			const char* b = p;
			size_t s = v.size();
			r = Visit(_ctx, SymbolType_Item, p, v);
			if (!r)
			{
				p = b;
				v.erase(v.begin() + s, v.end());
				break;
			}
		}
		r = true;
		return r;
	}

	bool Traverse_Space(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		const char* b = p;
		char c0 = *p++;
		r = (c0 == ' ');
		if (!r)
		{
			p = b;
			char c1 = *p++;
			r = (c1 == '\t');
			if (!r)
			{
				p = b;
				r = Traverse_EndOfLine(_ctx, p, v);
			}
		}
		return r;
	}

	bool Traverse_Spacing(Context& _ctx, CharItr& p, vector<Symbol>& v)
	{
		bool r = true;
		for (;;)
		{
			const char* b = p;
			r = Traverse_Space(_ctx, p, v);
			if (!r)
			{
				p = b;
				r = Traverse_Comment(_ctx, p, v);
			}
			if (!r)
			{
				p = b;
				break;
			}
		}
		r = true;
		return r;
	}

	void DebugPrint(ostream& _os, Context& _ctx, SymbolType _type, CharItr _pNode, int _tabs, int _maxLineSize)
	{
		vector<Symbol> children;
		CharItr pEnd = _pNode;
		if (!TraverseSymbol(_ctx, _type, pEnd, children))
			throw runtime_error(str(format("Parsing Failed for \"%1%\"") % SymbolName(_type)));

		int tabCount = _tabs;
		while (tabCount--)
			_os << "    ";

		_os << SymbolName(_type) << ": \"";

		size_t lineSize = 0;
		for (CharItr p = _pNode; p != pEnd; ++p)
		{
			_os << EscapeChar(*p);
			if (++lineSize >= _maxLineSize)
			{
				_os << "...";
				break;
			}
		}

		_os << "\"\n";

		for (vector<Symbol>::iterator i = children.begin(), iEnd = children.end(); i != iEnd; ++i)
			DebugPrint(_os, _ctx, i->type, i->value, _tabs + 1, _maxLineSize);
	}
}

PEGParser::Iterator::Iterator()
{
}

Iterator& PEGParser::Iterator::operator++()
{
	assert(mpSiblings);
	if (++mi == mpSiblings->end())
		mpSiblings.reset();
	return *this;
}

const Symbol& PEGParser::Iterator::operator*() const
{
	static const Symbol invalidSymbol = { SymbolTypeInvalid };
	if (mpSiblings)
		return *mi;
	else
		return invalidSymbol;
}

const Symbol* PEGParser::Iterator::operator->() const
{
	return &**this;
}

PEGParser::Iterator::Iterator(shared_ptr<Context> _pContext, shared_ptr< vector<Symbol> > _pSiblings)
: mpContext(_pContext)
, mpSiblings(_pSiblings)
{
	if (_pSiblings)
		mi = _pSiblings->begin();
}

Iterator PEGParser::Traverse(SymbolType _type, CharItr _text)
{
	shared_ptr<Context> pContext(new Context);
	CharItr p = _text;
	bool success = ParseSymbol(*pContext, _type, p);
    if (!success) --p;
    Symbol symbol = { _type, p - _text, _text, success };
    shared_ptr< vector<Symbol> > pSymbols(new vector<Symbol>(1, symbol));
    return Iterator(pContext, pSymbols);
}

Iterator PEGParser::Traverse(const Iterator& _iParent)
{
	if (_iParent.mpSiblings)
	{
		const Symbol& symbol = *_iParent.mi;
		CharItr p = symbol.value;
		vector<Symbol> children;
		bool r = TraverseSymbol(*_iParent.mpContext, symbol.type, p, children);
		assert(r && p == symbol.value + symbol.length);
		boost::shared_ptr< vector<Symbol> > pChildren;
		if (!children.empty())
		{
			pChildren.reset(new vector<Symbol>);
			pChildren->swap(children);
		}
		return Iterator(_iParent.mpContext, pChildren);
	}
	return Iterator();
}

void PEGParser::DebugPrint(ostream& _os, const Iterator& _i, int _tabs, int _maxLineSize)
{
	if (_i.mpSiblings)
		::DebugPrint(_os, *_i.mpContext, _i.mi->type, _i.mi->value, _tabs, _maxLineSize);
}

