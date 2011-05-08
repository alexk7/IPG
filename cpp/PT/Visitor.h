/*
 * Visitor.h
 *
 *  Created on: 17-08-2009
 *      Author: alexk7
 */

#ifndef VISITOR_H_
#define VISITOR_H_

#include "SymbolType.h"

class Symbol;
class Visitor;

class Visitor
{
public:
	virtual ~Visitor() {}
	virtual void Visit(Symbol* _symbol, SymbolType _type) = 0;
};

void VisitChildren(Symbol*& _symbol, SymbolType _type, Visitor& _visitor);

#endif /* VISITOR_H_ */
