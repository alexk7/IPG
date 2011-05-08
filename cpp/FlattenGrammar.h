/*
 *  FlattenGrammar.h
 *  ipg
 *
 *  Created by Alexandre Cossette on 09-12-12.
 *  Copyright 2009 alexk7. All rights reserved.
 *
 */

#ifndef FLATTENGRAMMAR_H_
#define FLATTENGRAMMAR_H_

class Grammar;
class Expression;

void FlattenGrammar(Grammar& _grammar);
//bool ReferenceAnyNode(const Defs& _defs, const Expression& _expr, std::set<std::string>& _visited);

#endif
