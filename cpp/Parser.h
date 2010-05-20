/*
 * Parser.h
 *
 *  Created on: 7-11-2009
 *      Author: alexk7
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "PT/SymbolType.h"

class Symbol;

Symbol* Parse(SymbolType _type, Symbol* _first);

#endif /* PARSER_H_ */
