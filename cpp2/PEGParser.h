#ifndef PEGPARSER_H
#define PEGPARSER_H

#include <bitset>
#include <vector>

struct PTNode;
typedef PTNode* PTNodeItr;

class Result
{
public:
    void Set(int _index, PTNodeItr _result);
    PTNodeItr Get(int _index) const;
    void Clear();

private:
    size_t GetPos(int _index) const;
    enum { PTNodeType_Count = 27 };
    std::bitset<PTNodeType_Count> mBitset;
    std::vector<PTNodeItr> mVector;
};

struct PTNode
{
    char value;
    Result end;
};

PTNodeItr Parse_AND(PTNodeItr _first);
PTNodeItr Parse_CLOSE(PTNodeItr _first);
PTNodeItr Parse_Char(PTNodeItr _first);
PTNodeItr Parse_Class(PTNodeItr _first);
PTNodeItr Parse_Comment(PTNodeItr _first);
PTNodeItr Parse_DOT(PTNodeItr _first);
PTNodeItr Parse_Definition(PTNodeItr _first);
PTNodeItr Parse_EndOfFile(PTNodeItr _first);
PTNodeItr Parse_EndOfLine(PTNodeItr _first);
PTNodeItr Parse_Expression(PTNodeItr _first);
PTNodeItr Parse_Grammar(PTNodeItr _first);
PTNodeItr Parse_Identifier(PTNodeItr _first);
PTNodeItr Parse_LEFTARROW(PTNodeItr _first);
PTNodeItr Parse_Literal(PTNodeItr _first);
PTNodeItr Parse_NOT(PTNodeItr _first);
PTNodeItr Parse_OPEN(PTNodeItr _first);
PTNodeItr Parse_PLUS(PTNodeItr _first);
PTNodeItr Parse_Prefix(PTNodeItr _first);
PTNodeItr Parse_Primary(PTNodeItr _first);
PTNodeItr Parse_QUESTION(PTNodeItr _first);
PTNodeItr Parse_Range(PTNodeItr _first);
PTNodeItr Parse_SLASH(PTNodeItr _first);
PTNodeItr Parse_STAR(PTNodeItr _first);
PTNodeItr Parse_Sequence(PTNodeItr _first);
PTNodeItr Parse_Space(PTNodeItr _first);
PTNodeItr Parse_Spacing(PTNodeItr _first);
PTNodeItr Parse_Suffix(PTNodeItr _first);
PTNodeItr Parse_Char_1(PTNodeItr _first);
PTNodeItr Parse_Char_1_1(PTNodeItr _first);
PTNodeItr Parse_Char_1_1_1(PTNodeItr _first);
PTNodeItr Parse_Char_1_1_1_1(PTNodeItr _first);
PTNodeItr Parse_Char_2(PTNodeItr _first);
PTNodeItr Parse_Char_2_1(PTNodeItr _first);
PTNodeItr Parse_Class_1(PTNodeItr _first);
PTNodeItr Parse_Class_1_1(PTNodeItr _first);
PTNodeItr Parse_Comment_1(PTNodeItr _first);
PTNodeItr Parse_Comment_1_1(PTNodeItr _first);
PTNodeItr Parse_EndOfFile_1(PTNodeItr _first);
PTNodeItr Parse_EndOfLine_1(PTNodeItr _first);
PTNodeItr Parse_Expression_1(PTNodeItr _first);
PTNodeItr Parse_Grammar_1(PTNodeItr _first);
PTNodeItr Parse_Identifier_1(PTNodeItr _first);
PTNodeItr Parse_Identifier_2(PTNodeItr _first);
PTNodeItr Parse_Literal_1(PTNodeItr _first);
PTNodeItr Parse_Literal_1_1(PTNodeItr _first);
PTNodeItr Parse_Literal_1_1_1(PTNodeItr _first);
PTNodeItr Parse_Literal_2(PTNodeItr _first);
PTNodeItr Parse_Literal_2_1(PTNodeItr _first);
PTNodeItr Parse_Literal_2_1_1(PTNodeItr _first);
PTNodeItr Parse_Prefix_1(PTNodeItr _first);
PTNodeItr Parse_Primary_1(PTNodeItr _first);
PTNodeItr Parse_Primary_1_1(PTNodeItr _first);
PTNodeItr Parse_Primary_2(PTNodeItr _first);
PTNodeItr Parse_Range_1(PTNodeItr _first);
PTNodeItr Parse_Sequence_1(PTNodeItr _first);
PTNodeItr Parse_Spacing_1(PTNodeItr _first);
PTNodeItr Parse_Suffix_1(PTNodeItr _first);

#endif
