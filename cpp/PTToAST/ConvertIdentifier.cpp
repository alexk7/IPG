/*
 * ConvertIdentifier.cpp
 *
 *  Created on: 07-04-2009
 *      Author: alexk7
 */

#include "../Common.h"
#include "ConvertIdentifier.h"
#include "../PT/Symbol.h"

void ConvertIdentifier(std::string& _identifier, Symbol* _symbol)
{
	assert(_symbol->result[SymbolType_IdentStart]);
	_identifier = _symbol->value;

	while ((++_symbol)->result[SymbolType_IdentCont])
		_identifier.push_back(_symbol->value);
}
