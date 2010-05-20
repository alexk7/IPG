/*
 * GenerateParser.h
 *
 *  Created on: 7-12-2009
 *      Author: alexk7
 */

class Grammar;

void GenerateParserSource(std::string _folder, std::string _name, const Grammar& _grammar);
void GenerateParserHeader(std::string _folder, std::string _name, const Grammar& _grammar);
