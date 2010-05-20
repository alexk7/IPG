Symbol* ParseNot(SymbolType _type, Symbol* _first)
{
	if (Parse(_type, _first))
		return NULL;
	return _first;
}

Symbol* ParseZeroOrMore(SymbolType _type, Symbol* _first)
{
	Symbol* result;
	while (result = Parse(_type, _first))
		_first = result;
	return _first;
}

Symbol* ParseOneOrMore(SymbolType _type, Symbol* _first)
{
	Symbol* result;
	if (!(result = Parse(_type, _first))) return NULL;
	return ParseZeroOrMore(_type, result);
}

Symbol* ParseOptional(SymbolType _type, Symbol* _first)
{
	Symbol* result;
	if (result = Parse(_type, _first))
		return result;
	return _first;
}

Symbol* ParseRange(char _rangeBegin, char _rangeEnd, Symbol* _symbol)
{
	if (_symbol->value < _rangeBegin) return NULL;
	if (_symbol->value > _rangeEnd) return NULL;
	return ++_symbol;
}

Symbol* ParseChar(char _char, Symbol* _symbol)
{
	if (_symbol->value != _char) return NULL;
	return ++_symbol;
}

Symbol* ParseString(const char* _string, Symbol* _first)
{
	while (*_string) {
		if (_first->value != *_string) return NULL;
		++_string;
		++_first;
	}
	return _first;
}

Symbol* ParseAnyChar(Symbol* _symbol)
{
	if (_symbol->value == 0) return NULL;
	return ++_symbol;
}
