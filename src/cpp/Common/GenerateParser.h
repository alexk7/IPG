#ifndef GENERATE_PARSER_H_
#define GENERATE_PARSER_H_

class Grammar;

void GenerateParserSource(std::string _ipgName, std::string _folder, std::string _name, const Grammar& _grammar);
void GenerateParserHeader(std::string _folder, std::string _name, const Grammar& _grammar);

#endif
