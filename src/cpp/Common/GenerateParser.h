#ifndef GENERATE_PARSER_H_
#define GENERATE_PARSER_H_

class Grammar;

void GenerateParser(std::string _srcPath, std::string _folder, std::string _name, const Grammar& _grammar);

#endif
