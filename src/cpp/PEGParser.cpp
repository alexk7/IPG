// This file was automatically generated by IPG on Thu Jul 21 16:22:53 2011
// (from /Users/alexk7/ipg/build/xcode/../../src/peg/peg.peg)
// DO NOT EDIT!
#ifndef BOOTSTRAP_TEST
#include <cassert>
#include <stdexcept>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <boost/format.hpp>
#endif

#ifndef BOOTSTRAP_TEST
#include <vector>
#include <bitset>
#include <boost/shared_ptr.hpp>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#endif

namespace PEGParser
{
	enum SymbolType
	{
		SymbolType_AND = 0,
		SymbolType_CLOSE = 1,
		SymbolType_Char = 2,
		SymbolType_Class = 3,
		SymbolType_Comment = 4,
		SymbolType_DOT = 5,
		SymbolType_Definition = 6,
		SymbolType_EndOfFile = 7,
		SymbolType_EndOfLine = 8,
		SymbolType_Expression = 9,
		SymbolType_Grammar = 10,
		SymbolType_Identifier = 11,
		SymbolType_LEFTARROW = 12,
		SymbolType_Literal = 13,
		SymbolType_NOT = 14,
		SymbolType_OPEN = 15,
		SymbolType_PLUS = 16,
		SymbolType_Prefix = 17,
		SymbolType_Primary = 18,
		SymbolType_QUESTION = 19,
		SymbolType_Range = 20,
		SymbolType_SLASH = 21,
		SymbolType_STAR = 22,
		SymbolType_Sequence = 23,
		SymbolType_Space = 24,
		SymbolType_Spacing = 25,
		SymbolType_Suffix = 26,
		SymbolTypeCount
	};

	typedef std::pair<SymbolType, const char*> Symbol;
	typedef std::vector<Symbol> Symbols;
	typedef std::tr1::unordered_map<const char*, const char*> EndMap;
	typedef std::tr1::unordered_set<const char*> FailSet;
	

	const char* SymbolName(SymbolType _type);

	class Parser
	{
	public:
		const char* Parse(SymbolType _type, const char* _symbol, bool _memoize = true);
		const char* Traverse(SymbolType _type, const char* _symbol, Symbols& _children, bool _memoize = true);
		void Print(std::ostream& _os, SymbolType _type, const char* _pNode, int _tabs = 0, int _maxLineSize = 100);

	private:
		const char* Visit(SymbolType _type, const char* _p, Symbols& _v);
		EndMap end[SymbolTypeCount];
		FailSet fail[SymbolTypeCount];
	};

	class Iterator
	{
	public:
		Iterator(SymbolType _type, const char* _pNode = 0);
		Iterator(const Iterator& _iOther);
		operator bool() const;
		Iterator& operator++();
		SymbolType GetType() const;
		const char* Begin() const;
		const char* End() const;
		Iterator GetChild(SymbolType _childT);
		Iterator GetNext(SymbolType _childT);
		void Print(std::ostream& _os, int _tabs = 0, int _maxLineSize = 100);

	private:
		Iterator(boost::shared_ptr<Parser> _pParser, boost::shared_ptr<Symbols> _pSiblings, SymbolType _childType);
		Iterator(const Iterator& _iOther, SymbolType _childType);
		void GoToNext(SymbolType _childType);
		void SkipSiblingsWithWrongType(SymbolType _childType);
		boost::shared_ptr<Symbols> GetChildren();

		SymbolType mType;
		const char* mpNode;
		boost::shared_ptr<Symbols> mpSiblings;
		Symbols::iterator miCurrent;
		boost::shared_ptr<Symbols> mpChildren;
		boost::shared_ptr<Parser> mpParser;
	};

	std::ostream& operator<<(std::ostream& _os, const Iterator& _i);
}

namespace
{
	struct EscapeChar
	{
		EscapeChar(char _c);
		char c;
	};

	EscapeChar::EscapeChar(char _c) : c(_c)
	{
	}

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
	}
}

const char* PEGParser::SymbolName(PEGParser::SymbolType _type)
{
	switch (_type)
	{
		case SymbolType_AND: return "AND";
		case SymbolType_CLOSE: return "CLOSE";
		case SymbolType_Char: return "Char";
		case SymbolType_Class: return "Class";
		case SymbolType_Comment: return "Comment";
		case SymbolType_DOT: return "DOT";
		case SymbolType_Definition: return "Definition";
		case SymbolType_EndOfFile: return "EndOfFile";
		case SymbolType_EndOfLine: return "EndOfLine";
		case SymbolType_Expression: return "Expression";
		case SymbolType_Grammar: return "Grammar";
		case SymbolType_Identifier: return "Identifier";
		case SymbolType_LEFTARROW: return "LEFTARROW";
		case SymbolType_Literal: return "Literal";
		case SymbolType_NOT: return "NOT";
		case SymbolType_OPEN: return "OPEN";
		case SymbolType_PLUS: return "PLUS";
		case SymbolType_Prefix: return "Prefix";
		case SymbolType_Primary: return "Primary";
		case SymbolType_QUESTION: return "QUESTION";
		case SymbolType_Range: return "Range";
		case SymbolType_SLASH: return "SLASH";
		case SymbolType_STAR: return "STAR";
		case SymbolType_Sequence: return "Sequence";
		case SymbolType_Space: return "Space";
		case SymbolType_Spacing: return "Spacing";
		case SymbolType_Suffix: return "Suffix";
		default: throw std::runtime_error(str(boost::format("Invalid Symbol Type: %1%") % _type));
	}
}

const char* PEGParser::Parser::Parse(SymbolType _type, const char* _pBegin, bool _memoize)
{
	const char* p = _pBegin;
	switch (_type)
	{
		case SymbolType_AND:
		{
			if (fail[SymbolType_AND].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_AND].find(_pBegin);
			if (i != end[SymbolType_AND].end())
				return i->second;
			if (*p == '&')
				++p;
			else
				p = 0;
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 0);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_AND][_pBegin] = p;
				else
					fail[SymbolType_AND].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_CLOSE:
		{
			if (fail[SymbolType_CLOSE].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_CLOSE].find(_pBegin);
			if (i != end[SymbolType_CLOSE].end())
				return i->second;
			if (*p == ')')
				++p;
			else
				p = 0;
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 0);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_CLOSE][_pBegin] = p;
				else
					fail[SymbolType_CLOSE].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Char:
		{
			if (fail[SymbolType_Char].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Char].find(_pBegin);
			if (i != end[SymbolType_Char].end())
				return i->second;
			const char* b = p;
			if (*p == '\\')
				++p;
			else
				p = 0;
			if (p)
			{
				const char* b2 = p;
				if (*p == 'n')
					++p;
				else
					p = 0;
				if (!p)
				{
					p = b2;
					if (*p == 'r')
						++p;
					else
						p = 0;
					if (!p)
					{
						p = b2;
						if (*p == 't')
							++p;
						else
							p = 0;
						if (!p)
						{
							p = b2;
							if (*p == '\'')
								++p;
							else
								p = 0;
							if (!p)
							{
								p = b2;
								if (*p == '\"')
									++p;
								else
									p = 0;
								if (!p)
								{
									p = b2;
									if (*p == '[')
										++p;
									else
										p = 0;
									if (!p)
									{
										p = b2;
										if (*p == ']')
											++p;
										else
											p = 0;
										if (!p)
										{
											p = b2;
											if (*p == '\\')
												++p;
											else
												p = 0;
											if (!p)
											{
												p = b2;
												if (*p >= '1' && *p <= '9')
													++p;
												else
													p = 0;
												if (p)
												{
													for (;;)
													{
														const char* b3 = p;
														if (*p >= '0' && *p <= '9')
															++p;
														else
															p = 0;
														if (!p)
														{
															p = b3;
															break;
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
				}
			}
			if (!p)
			{
				p = b;
				if (*p == '\\')
					++p;
				else
					p = 0;
				if (p) p = 0; else p = b;
				if (p)
				{
					if (*p != 0)
						++p;
					else
						p = 0;
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Char][_pBegin] = p;
				else
					fail[SymbolType_Char].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Class:
		{
			if (fail[SymbolType_Class].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Class].find(_pBegin);
			if (i != end[SymbolType_Class].end())
				return i->second;
			if (*p == '[')
				++p;
			else
				p = 0;
			if (p)
			{
				for (;;)
				{
					const char* b = p;
					const char* b2 = p;
					if (*p == ']')
						++p;
					else
						p = 0;
					if (p) p = 0; else p = b2;
					if (p)
					{
						p = Parse(SymbolType_Range, p, 1);
					}
					if (!p)
					{
						p = b;
						break;
					}
				}
				if (p)
				{
					if (*p == ']')
						++p;
					else
						p = 0;
					if (p)
					{
						p = Parse(SymbolType_Spacing, p, 1);
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Class][_pBegin] = p;
				else
					fail[SymbolType_Class].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Comment:
		{
			if (fail[SymbolType_Comment].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Comment].find(_pBegin);
			if (i != end[SymbolType_Comment].end())
				return i->second;
			if (*p == '#')
				++p;
			else
				p = 0;
			if (p)
			{
				for (;;)
				{
					const char* b = p;
					const char* b2 = p;
					p = Parse(SymbolType_EndOfLine, p, 1);
					if (p) p = 0; else p = b2;
					if (p)
					{
						if (*p != 0)
							++p;
						else
							p = 0;
					}
					if (!p)
					{
						p = b;
						break;
					}
				}
				if (p)
				{
					p = Parse(SymbolType_EndOfLine, p, 0);
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Comment][_pBegin] = p;
				else
					fail[SymbolType_Comment].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_DOT:
		{
			if (fail[SymbolType_DOT].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_DOT].find(_pBegin);
			if (i != end[SymbolType_DOT].end())
				return i->second;
			if (*p == '.')
				++p;
			else
				p = 0;
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 0);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_DOT][_pBegin] = p;
				else
					fail[SymbolType_DOT].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Definition:
		{
			if (fail[SymbolType_Definition].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Definition].find(_pBegin);
			if (i != end[SymbolType_Definition].end())
				return i->second;
			p = Parse(SymbolType_Identifier, p, 1);
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 1);
				if (p)
				{
					p = Parse(SymbolType_LEFTARROW, p, 1);
					if (p)
					{
						p = Parse(SymbolType_Expression, p, 1);
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Definition][_pBegin] = p;
				else
					fail[SymbolType_Definition].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_EndOfFile:
		{
			if (fail[SymbolType_EndOfFile].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_EndOfFile].find(_pBegin);
			if (i != end[SymbolType_EndOfFile].end())
				return i->second;
			const char* b = p;
			if (*p != 0)
				++p;
			else
				p = 0;
			if (p) p = 0; else p = b;
			if (_memoize)
			{
				if (p)
					end[SymbolType_EndOfFile][_pBegin] = p;
				else
					fail[SymbolType_EndOfFile].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_EndOfLine:
		{
			if (fail[SymbolType_EndOfLine].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_EndOfLine].find(_pBegin);
			if (i != end[SymbolType_EndOfLine].end())
				return i->second;
			const char* b = p;
			if (*p == '\r')
				++p;
			else
				p = 0;
			if (p)
			{
				if (*p == '\n')
					++p;
				else
					p = 0;
			}
			if (!p)
			{
				p = b;
				if (*p == '\n')
					++p;
				else
					p = 0;
				if (!p)
				{
					p = b;
					if (*p == '\r')
						++p;
					else
						p = 0;
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_EndOfLine][_pBegin] = p;
				else
					fail[SymbolType_EndOfLine].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Expression:
		{
			if (fail[SymbolType_Expression].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Expression].find(_pBegin);
			if (i != end[SymbolType_Expression].end())
				return i->second;
			p = Parse(SymbolType_Sequence, p, 1);
			if (p)
			{
				for (;;)
				{
					const char* b = p;
					p = Parse(SymbolType_SLASH, p, 1);
					if (p)
					{
						p = Parse(SymbolType_Sequence, p, 1);
					}
					if (!p)
					{
						p = b;
						break;
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Expression][_pBegin] = p;
				else
					fail[SymbolType_Expression].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Grammar:
		{
			if (fail[SymbolType_Grammar].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Grammar].find(_pBegin);
			if (i != end[SymbolType_Grammar].end())
				return i->second;
			p = Parse(SymbolType_Spacing, p, 1);
			if (p)
			{
				p = Parse(SymbolType_Definition, p, 1);
				if (p)
				{
					for (;;)
					{
						const char* b = p;
						p = Parse(SymbolType_Definition, p, 1);
						if (!p)
						{
							p = b;
							break;
						}
					}
					if (p)
					{
						p = Parse(SymbolType_EndOfFile, p, 1);
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Grammar][_pBegin] = p;
				else
					fail[SymbolType_Grammar].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Identifier:
		{
			if (fail[SymbolType_Identifier].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Identifier].find(_pBegin);
			if (i != end[SymbolType_Identifier].end())
				return i->second;
			const char* b = p;
			if (*p >= 'a' && *p <= 'z')
				++p;
			else
				p = 0;
			if (!p)
			{
				p = b;
				if (*p >= 'A' && *p <= 'Z')
					++p;
				else
					p = 0;
				if (!p)
				{
					p = b;
					if (*p == '_')
						++p;
					else
						p = 0;
				}
			}
			if (p)
			{
				for (;;)
				{
					const char* b2 = p;
					const char* b3 = p;
					if (*p >= 'a' && *p <= 'z')
						++p;
					else
						p = 0;
					if (!p)
					{
						p = b3;
						if (*p >= 'A' && *p <= 'Z')
							++p;
						else
							p = 0;
						if (!p)
						{
							p = b3;
							if (*p >= '0' && *p <= '9')
								++p;
							else
								p = 0;
							if (!p)
							{
								p = b3;
								if (*p == '_')
									++p;
								else
									p = 0;
							}
						}
					}
					if (!p)
					{
						p = b2;
						break;
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Identifier][_pBegin] = p;
				else
					fail[SymbolType_Identifier].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_LEFTARROW:
		{
			if (fail[SymbolType_LEFTARROW].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_LEFTARROW].find(_pBegin);
			if (i != end[SymbolType_LEFTARROW].end())
				return i->second;
			if (*p == '<')
				++p;
			else
				p = 0;
			if (p)
			{
				const char* b = p;
				if (*p == '-')
					++p;
				else
					p = 0;
				if (!p)
				{
					p = b;
					if (*p == '=')
						++p;
					else
						p = 0;
					if (!p)
					{
						p = b;
						if (*p == '<')
							++p;
						else
							p = 0;
					}
				}
				if (p)
				{
					p = Parse(SymbolType_Spacing, p, 1);
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_LEFTARROW][_pBegin] = p;
				else
					fail[SymbolType_LEFTARROW].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Literal:
		{
			if (fail[SymbolType_Literal].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Literal].find(_pBegin);
			if (i != end[SymbolType_Literal].end())
				return i->second;
			const char* b = p;
			if (*p == '\'')
				++p;
			else
				p = 0;
			if (p)
			{
				for (;;)
				{
					const char* b2 = p;
					const char* b3 = p;
					if (*p == '\'')
						++p;
					else
						p = 0;
					if (p) p = 0; else p = b3;
					if (p)
					{
						p = Parse(SymbolType_Char, p, 1);
					}
					if (!p)
					{
						p = b2;
						break;
					}
				}
				if (p)
				{
					if (*p == '\'')
						++p;
					else
						p = 0;
					if (p)
					{
						p = Parse(SymbolType_Spacing, p, 1);
					}
				}
			}
			if (!p)
			{
				p = b;
				if (*p == '\"')
					++p;
				else
					p = 0;
				if (p)
				{
					for (;;)
					{
						const char* b4 = p;
						const char* b5 = p;
						if (*p == '\"')
							++p;
						else
							p = 0;
						if (p) p = 0; else p = b5;
						if (p)
						{
							p = Parse(SymbolType_Char, p, 1);
						}
						if (!p)
						{
							p = b4;
							break;
						}
					}
					if (p)
					{
						if (*p == '\"')
							++p;
						else
							p = 0;
						if (p)
						{
							p = Parse(SymbolType_Spacing, p, 1);
						}
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Literal][_pBegin] = p;
				else
					fail[SymbolType_Literal].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_NOT:
		{
			if (fail[SymbolType_NOT].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_NOT].find(_pBegin);
			if (i != end[SymbolType_NOT].end())
				return i->second;
			if (*p == '!')
				++p;
			else
				p = 0;
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 0);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_NOT][_pBegin] = p;
				else
					fail[SymbolType_NOT].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_OPEN:
		{
			if (fail[SymbolType_OPEN].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_OPEN].find(_pBegin);
			if (i != end[SymbolType_OPEN].end())
				return i->second;
			if (*p == '(')
				++p;
			else
				p = 0;
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 0);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_OPEN][_pBegin] = p;
				else
					fail[SymbolType_OPEN].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_PLUS:
		{
			if (fail[SymbolType_PLUS].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_PLUS].find(_pBegin);
			if (i != end[SymbolType_PLUS].end())
				return i->second;
			if (*p == '+')
				++p;
			else
				p = 0;
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 0);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_PLUS][_pBegin] = p;
				else
					fail[SymbolType_PLUS].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Prefix:
		{
			if (fail[SymbolType_Prefix].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Prefix].find(_pBegin);
			if (i != end[SymbolType_Prefix].end())
				return i->second;
			const char* b = p;
			p = Parse(SymbolType_AND, p, 1);
			if (!p)
			{
				p = b;
				p = Parse(SymbolType_NOT, p, 1);
				if (!p)
				{
					p = b;
				}
			}
			if (p)
			{
				p = Parse(SymbolType_Suffix, p, 1);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Prefix][_pBegin] = p;
				else
					fail[SymbolType_Prefix].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Primary:
		{
			if (fail[SymbolType_Primary].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Primary].find(_pBegin);
			if (i != end[SymbolType_Primary].end())
				return i->second;
			const char* b = p;
			p = Parse(SymbolType_Identifier, p, 1);
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 1);
				if (p)
				{
					const char* b2 = p;
					p = Parse(SymbolType_LEFTARROW, p, 1);
					if (p) p = 0; else p = b2;
				}
			}
			if (!p)
			{
				p = b;
				p = Parse(SymbolType_OPEN, p, 1);
				if (p)
				{
					p = Parse(SymbolType_Expression, p, 1);
					if (p)
					{
						p = Parse(SymbolType_CLOSE, p, 1);
					}
				}
				if (!p)
				{
					p = b;
					p = Parse(SymbolType_Literal, p, 1);
					if (!p)
					{
						p = b;
						p = Parse(SymbolType_Class, p, 1);
						if (!p)
						{
							p = b;
							p = Parse(SymbolType_DOT, p, 1);
						}
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Primary][_pBegin] = p;
				else
					fail[SymbolType_Primary].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_QUESTION:
		{
			if (fail[SymbolType_QUESTION].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_QUESTION].find(_pBegin);
			if (i != end[SymbolType_QUESTION].end())
				return i->second;
			if (*p == '?')
				++p;
			else
				p = 0;
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 0);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_QUESTION][_pBegin] = p;
				else
					fail[SymbolType_QUESTION].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Range:
		{
			if (fail[SymbolType_Range].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Range].find(_pBegin);
			if (i != end[SymbolType_Range].end())
				return i->second;
			p = Parse(SymbolType_Char, p, 1);
			if (p)
			{
				const char* b = p;
				if (*p == '-')
					++p;
				else
					p = 0;
				if (p)
				{
					p = Parse(SymbolType_Char, p, 1);
				}
				if (!p)
				{
					p = b;
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Range][_pBegin] = p;
				else
					fail[SymbolType_Range].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_SLASH:
		{
			if (fail[SymbolType_SLASH].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_SLASH].find(_pBegin);
			if (i != end[SymbolType_SLASH].end())
				return i->second;
			if (*p == '/')
				++p;
			else
				p = 0;
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 0);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_SLASH][_pBegin] = p;
				else
					fail[SymbolType_SLASH].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_STAR:
		{
			if (fail[SymbolType_STAR].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_STAR].find(_pBegin);
			if (i != end[SymbolType_STAR].end())
				return i->second;
			if (*p == '*')
				++p;
			else
				p = 0;
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 0);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_STAR][_pBegin] = p;
				else
					fail[SymbolType_STAR].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Sequence:
		{
			if (fail[SymbolType_Sequence].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Sequence].find(_pBegin);
			if (i != end[SymbolType_Sequence].end())
				return i->second;
			for (;;)
			{
				const char* b = p;
				p = Parse(SymbolType_Prefix, p, 1);
				if (!p)
				{
					p = b;
					break;
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Sequence][_pBegin] = p;
				else
					fail[SymbolType_Sequence].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Space:
		{
			if (fail[SymbolType_Space].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Space].find(_pBegin);
			if (i != end[SymbolType_Space].end())
				return i->second;
			const char* b = p;
			if (*p == ' ')
				++p;
			else
				p = 0;
			if (!p)
			{
				p = b;
				if (*p == '\t')
					++p;
				else
					p = 0;
				if (!p)
				{
					p = b;
					p = Parse(SymbolType_EndOfLine, p, 0);
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Space][_pBegin] = p;
				else
					fail[SymbolType_Space].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Spacing:
		{
			if (fail[SymbolType_Spacing].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Spacing].find(_pBegin);
			if (i != end[SymbolType_Spacing].end())
				return i->second;
			for (;;)
			{
				const char* b = p;
				const char* b2 = p;
				p = Parse(SymbolType_Space, p, 1);
				if (!p)
				{
					p = b2;
					p = Parse(SymbolType_Comment, p, 1);
				}
				if (!p)
				{
					p = b;
					break;
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Spacing][_pBegin] = p;
				else
					fail[SymbolType_Spacing].insert(_pBegin);
			}
		  return p;
		}

		case SymbolType_Suffix:
		{
			if (fail[SymbolType_Suffix].count(_pBegin))
				return 0;
			EndMap::iterator i = end[SymbolType_Suffix].find(_pBegin);
			if (i != end[SymbolType_Suffix].end())
				return i->second;
			p = Parse(SymbolType_Primary, p, 1);
			if (p)
			{
				const char* b = p;
				p = Parse(SymbolType_QUESTION, p, 1);
				if (!p)
				{
					p = b;
					p = Parse(SymbolType_STAR, p, 1);
					if (!p)
					{
						p = b;
						p = Parse(SymbolType_PLUS, p, 1);
						if (!p)
						{
							p = b;
						}
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Suffix][_pBegin] = p;
				else
					fail[SymbolType_Suffix].insert(_pBegin);
			}
		  return p;
		}

		default:
			assert(false);
			return 0;
	}
}

const char* PEGParser::Parser::Traverse(PEGParser::SymbolType _type, const char* _pBegin, PEGParser::Symbols& v, bool _memoize)
{
	const char* p = _pBegin;
	switch (_type)
	{
		case SymbolType_Class:
		{
			if (fail[SymbolType_Class].count(_pBegin))
				return 0;
			if (*p == '[')
				++p;
			else
				p = 0;
			if (p)
			{
				for (;;)
				{
					const char* b = p;
					size_t s = v.size();
					const char* b2 = p;
					if (*p == ']')
						++p;
					else
						p = 0;
					if (p) p = 0; else p = b2;
					if (p)
					{
						p = Visit(SymbolType_Range, p, v);
					}
					if (!p)
					{
						p = b;
						v.erase(v.begin() + s, v.end());
						break;
					}
				}
				if (p)
				{
					if (*p == ']')
						++p;
					else
						p = 0;
					if (p)
					{
						p = Parse(SymbolType_Spacing, p, 1);
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Class][_pBegin] = p;
				else
					fail[SymbolType_Class].insert(_pBegin);
			}
			return p;
		}

		case SymbolType_Definition:
		{
			if (fail[SymbolType_Definition].count(_pBegin))
				return 0;
			p = Visit(SymbolType_Identifier, p, v);
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 1);
				if (p)
				{
					p = Visit(SymbolType_LEFTARROW, p, v);
					if (p)
					{
						p = Visit(SymbolType_Expression, p, v);
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Definition][_pBegin] = p;
				else
					fail[SymbolType_Definition].insert(_pBegin);
			}
			return p;
		}

		case SymbolType_Expression:
		{
			if (fail[SymbolType_Expression].count(_pBegin))
				return 0;
			p = Visit(SymbolType_Sequence, p, v);
			if (p)
			{
				for (;;)
				{
					const char* b = p;
					size_t s = v.size();
					p = Parse(SymbolType_SLASH, p, 1);
					if (p)
					{
						p = Visit(SymbolType_Sequence, p, v);
					}
					if (!p)
					{
						p = b;
						v.erase(v.begin() + s, v.end());
						break;
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Expression][_pBegin] = p;
				else
					fail[SymbolType_Expression].insert(_pBegin);
			}
			return p;
		}

		case SymbolType_Grammar:
		{
			if (fail[SymbolType_Grammar].count(_pBegin))
				return 0;
			p = Parse(SymbolType_Spacing, p, 1);
			if (p)
			{
				p = Visit(SymbolType_Definition, p, v);
				if (p)
				{
					for (;;)
					{
						const char* b = p;
						size_t s = v.size();
						p = Visit(SymbolType_Definition, p, v);
						if (!p)
						{
							p = b;
							v.erase(v.begin() + s, v.end());
							break;
						}
					}
					if (p)
					{
						p = Parse(SymbolType_EndOfFile, p, 1);
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Grammar][_pBegin] = p;
				else
					fail[SymbolType_Grammar].insert(_pBegin);
			}
			return p;
		}

		case SymbolType_Literal:
		{
			if (fail[SymbolType_Literal].count(_pBegin))
				return 0;
			const char* b = p;
			size_t s = v.size();
			if (*p == '\'')
				++p;
			else
				p = 0;
			if (p)
			{
				for (;;)
				{
					const char* b2 = p;
					size_t s2 = v.size();
					const char* b3 = p;
					if (*p == '\'')
						++p;
					else
						p = 0;
					if (p) p = 0; else p = b3;
					if (p)
					{
						p = Visit(SymbolType_Char, p, v);
					}
					if (!p)
					{
						p = b2;
						v.erase(v.begin() + s2, v.end());
						break;
					}
				}
				if (p)
				{
					if (*p == '\'')
						++p;
					else
						p = 0;
					if (p)
					{
						p = Parse(SymbolType_Spacing, p, 1);
					}
				}
			}
			if (!p)
			{
				p = b;
				v.erase(v.begin() + s, v.end());
				if (*p == '\"')
					++p;
				else
					p = 0;
				if (p)
				{
					for (;;)
					{
						const char* b4 = p;
						size_t s4 = v.size();
						const char* b5 = p;
						if (*p == '\"')
							++p;
						else
							p = 0;
						if (p) p = 0; else p = b5;
						if (p)
						{
							p = Visit(SymbolType_Char, p, v);
						}
						if (!p)
						{
							p = b4;
							v.erase(v.begin() + s4, v.end());
							break;
						}
					}
					if (p)
					{
						if (*p == '\"')
							++p;
						else
							p = 0;
						if (p)
						{
							p = Parse(SymbolType_Spacing, p, 1);
						}
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Literal][_pBegin] = p;
				else
					fail[SymbolType_Literal].insert(_pBegin);
			}
			return p;
		}

		case SymbolType_Prefix:
		{
			if (fail[SymbolType_Prefix].count(_pBegin))
				return 0;
			const char* b = p;
			p = Parse(SymbolType_AND, p, 1);
			if (!p)
			{
				p = b;
				p = Parse(SymbolType_NOT, p, 1);
				if (!p)
				{
					p = b;
				}
			}
			if (p)
			{
				p = Visit(SymbolType_Suffix, p, v);
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Prefix][_pBegin] = p;
				else
					fail[SymbolType_Prefix].insert(_pBegin);
			}
			return p;
		}

		case SymbolType_Primary:
		{
			if (fail[SymbolType_Primary].count(_pBegin))
				return 0;
			const char* b = p;
			size_t s = v.size();
			p = Visit(SymbolType_Identifier, p, v);
			if (p)
			{
				p = Parse(SymbolType_Spacing, p, 1);
				if (p)
				{
					const char* b2 = p;
					p = Parse(SymbolType_LEFTARROW, p, 1);
					if (p) p = 0; else p = b2;
				}
			}
			if (!p)
			{
				p = b;
				v.erase(v.begin() + s, v.end());
				p = Parse(SymbolType_OPEN, p, 1);
				if (p)
				{
					p = Visit(SymbolType_Expression, p, v);
					if (p)
					{
						p = Parse(SymbolType_CLOSE, p, 1);
					}
				}
				if (!p)
				{
					p = b;
					v.erase(v.begin() + s, v.end());
					p = Visit(SymbolType_Literal, p, v);
					if (!p)
					{
						p = b;
						v.erase(v.begin() + s, v.end());
						p = Visit(SymbolType_Class, p, v);
						if (!p)
						{
							p = b;
							v.erase(v.begin() + s, v.end());
							p = Parse(SymbolType_DOT, p, 1);
						}
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Primary][_pBegin] = p;
				else
					fail[SymbolType_Primary].insert(_pBegin);
			}
			return p;
		}

		case SymbolType_Range:
		{
			if (fail[SymbolType_Range].count(_pBegin))
				return 0;
			p = Visit(SymbolType_Char, p, v);
			if (p)
			{
				const char* b = p;
				size_t s = v.size();
				if (*p == '-')
					++p;
				else
					p = 0;
				if (p)
				{
					p = Visit(SymbolType_Char, p, v);
				}
				if (!p)
				{
					p = b;
					v.erase(v.begin() + s, v.end());
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Range][_pBegin] = p;
				else
					fail[SymbolType_Range].insert(_pBegin);
			}
			return p;
		}

		case SymbolType_Sequence:
		{
			if (fail[SymbolType_Sequence].count(_pBegin))
				return 0;
			for (;;)
			{
				const char* b = p;
				size_t s = v.size();
				p = Visit(SymbolType_Prefix, p, v);
				if (!p)
				{
					p = b;
					v.erase(v.begin() + s, v.end());
					break;
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Sequence][_pBegin] = p;
				else
					fail[SymbolType_Sequence].insert(_pBegin);
			}
			return p;
		}

		case SymbolType_Suffix:
		{
			if (fail[SymbolType_Suffix].count(_pBegin))
				return 0;
			p = Visit(SymbolType_Primary, p, v);
			if (p)
			{
				const char* b = p;
				p = Parse(SymbolType_QUESTION, p, 1);
				if (!p)
				{
					p = b;
					p = Parse(SymbolType_STAR, p, 1);
					if (!p)
					{
						p = b;
						p = Parse(SymbolType_PLUS, p, 1);
						if (!p)
						{
							p = b;
						}
					}
				}
			}
			if (_memoize)
			{
				if (p)
					end[SymbolType_Suffix][_pBegin] = p;
				else
					fail[SymbolType_Suffix].insert(_pBegin);
			}
			return p;
		}

		case SymbolType_AND:
		case SymbolType_CLOSE:
		case SymbolType_Char:
		case SymbolType_Comment:
		case SymbolType_DOT:
		case SymbolType_EndOfFile:
		case SymbolType_EndOfLine:
		case SymbolType_Identifier:
		case SymbolType_LEFTARROW:
		case SymbolType_NOT:
		case SymbolType_OPEN:
		case SymbolType_PLUS:
		case SymbolType_QUESTION:
		case SymbolType_SLASH:
		case SymbolType_STAR:
		case SymbolType_Space:
		case SymbolType_Spacing:
			return Parse(_type, p);

		default:
			assert(false);
			return 0;
	}
}

void PEGParser::Parser::Print(std::ostream& _os, PEGParser::SymbolType _type, const char* _pNode, int _tabs, int _maxLineSize)
{
	Symbols children;
	const char* pEnd = Traverse(_type, _pNode, children);
	if (!pEnd)
		throw std::runtime_error(str(boost::format("Parsing Failed for \"%1%\"") % SymbolName(_type)));

	int tabCount = _tabs;
	while (tabCount--)
		_os << "    ";

	_os << SymbolName(_type) << ": \"";

	size_t lineSize = 0;
	for (const char* p = _pNode; p != pEnd; ++p)
	{
		_os << EscapeChar(*p);
		if (++lineSize >= _maxLineSize)
		{
			_os << "...";
			break;
		}
	}

	_os << "\"\n";

	for (Symbols::iterator i = children.begin(), iEnd = children.end(); i != iEnd; ++i)
		Print(_os, i->first, i->second, _tabs + 1, _maxLineSize);
//*
	if (_tabs == 0)
	{
		_os << "Memo Count:\n";
		for (size_t k = 0; k < SymbolTypeCount; ++k)
		{
			_os << boost::format("%1% %|20t|End: %2% %|40t|Fail: %3%\n") % SymbolName(SymbolType(k)) % end[k].size() % fail[k].size();
		}
	}
//*/
}

const char* PEGParser::Parser::Visit(PEGParser::SymbolType _type, const char* _p, PEGParser::Symbols& _v)
{
	const char* pEnd = Parse(_type, _p);
	if (pEnd)
		_v.push_back(Symbol(_type, _p));
	return pEnd;
}

PEGParser::Iterator::Iterator(PEGParser::SymbolType _type, const char* _pNode)
:	mType(_type)
,	mpNode(_pNode)
, mpSiblings(new Symbols)
, mpParser(new Parser)
{
	if (mpNode && End())
		mpSiblings->push_back(Symbol(mType, mpNode));

	miCurrent = mpSiblings->begin();				
}

PEGParser::Iterator::Iterator(const PEGParser::Iterator& _iOther)
: mType(_iOther.mType)
,	mpNode(_iOther.mpNode)
, mpSiblings(_iOther.mpSiblings)
, miCurrent(_iOther.miCurrent)
,	mpParser(_iOther.mpParser)
{
}

PEGParser::Iterator::operator bool() const
{
	return mpNode != 0;
}

PEGParser::Iterator& PEGParser::Iterator::operator++()
{
	mpChildren.reset();
	GoToNext(mType);
	return *this;
}

PEGParser::SymbolType PEGParser::Iterator::GetType() const
{
	return mType;
}

const char* PEGParser::Iterator::Begin() const
{
	assert(mpNode);
	return mpNode;
}

const char* PEGParser::Iterator::End() const
{
	assert(mpNode);
	return mpParser->Parse(mType, mpNode);
}

PEGParser::Iterator PEGParser::Iterator::GetChild(PEGParser::SymbolType _childT)
{
	assert(mpNode != 0);
	return Iterator(mpParser, GetChildren(), _childT);
}

PEGParser::Iterator PEGParser::Iterator::GetNext(PEGParser::SymbolType _childT)
{
	assert(mpNode != 0);
	return Iterator(*this, _childT);
}

void PEGParser::Iterator::Print(std::ostream& _os, int _tabs, int _maxLineSize)
{
	mpParser->Print(_os, mType, mpNode, _tabs, _maxLineSize);
}

PEGParser::Iterator::Iterator(boost::shared_ptr<PEGParser::Parser> _pParser, boost::shared_ptr<PEGParser::Symbols> _pSiblings, PEGParser::SymbolType _childType)
: mType(_childType)
, mpSiblings(_pSiblings)
, miCurrent(_pSiblings->begin())
,	mpParser(_pParser)
{
	SkipSiblingsWithWrongType(_childType);
}

PEGParser::Iterator::Iterator(const PEGParser::Iterator& _iOther, PEGParser::SymbolType _childType)
: mType(_childType)
,	mpSiblings(_iOther.mpSiblings)
,	miCurrent(_iOther.miCurrent)
,	mpParser(_iOther.mpParser)
{
	SkipSiblingsWithWrongType(_childType);
}

void PEGParser::Iterator::GoToNext(PEGParser::SymbolType _childType)
{
	++miCurrent;
	SkipSiblingsWithWrongType(_childType);
}

void PEGParser::Iterator::SkipSiblingsWithWrongType(PEGParser::SymbolType _childType)
{
	Symbols::iterator iEnd = mpSiblings->end();
	while (miCurrent != iEnd && miCurrent->first != _childType)
		++miCurrent;
	mpNode = (miCurrent != iEnd) ? miCurrent->second : 0 ;
}

boost::shared_ptr<PEGParser::Symbols> PEGParser::Iterator::GetChildren()
{
	if (!mpChildren)
	{
		Symbols children;
		mpParser->Traverse(mType, mpNode, children);
		mpChildren.reset(new Symbols);
		mpChildren->swap(children);			
	}
	return mpChildren;
}

std::ostream& PEGParser::operator<<(std::ostream& _os, const PEGParser::Iterator& _i)
{
	if (_i)
	{
		for (const char *p = _i.Begin(), *pEnd = _i.End(); p != pEnd; ++p)
			_os.put(*p);
	}
	return _os;
}

