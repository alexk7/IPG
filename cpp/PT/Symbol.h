/*
 * Symbol.h
 *
 *  Created on: 07-04-2009
 *      Author: alexk7
 */

#ifndef SYMBOL_H_
#define SYMBOL_H_

#include "SymbolType.h"
#include "../Containers/SparseArray.h"

class Symbol
{
public:
    char value;
    SparseArray<Symbol*, SymbolType_Count> result;
};

#endif /* SYMBOL_H_ */
