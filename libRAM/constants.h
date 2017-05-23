
#ifndef CONFIG_H
#define CONFIG_H


#include <vector>
#include <cmath>

namespace ramConfig
{
    
    extern int instructionLength;
    extern int N;
    extern int logN;
//    extern int argLength;
    extern int leafIdLength;
    extern int dataLength;
    extern int elementsPerNode;

//    extern std::vector<int> universalInstructionInput_INSTRUCTION;
//    extern std::vector<int> universalInstructionInput_ARG1;
//    extern std::vector<int> universalInstructionInput_ARG2;
//    extern std::vector<int> universalInstructionInput_ARG_DATA1;
//    extern std::vector<int> universalInstructionInput_ARG_DATA2;
//    extern std::vector<int> universalInstructionInput_PC;
//    extern std::vector<int> universalInstructionInput_RDATA;
//    extern std::vector<int> universalInstructionOutput_PC;
//    extern std::vector<int> universalInstructionOutput_RDATA;
//    extern std::vector<int> universalInstructionOutput_WDATA;
//    extern std::vector<int> universalInstructionOutput_WDATA_ADDR;
//
//    extern std::vector<int> lookupInput_REAL_DATA_ID;
//    extern std::vector<int> lookupInput_TABLE;
//    extern std::vector<int> lookupOutput_REAL_DATA_ID;
//    extern std::vector<int> lookupOutput_TABLE;
//    extern std::vector<int> lookupOutput_NEW_LEAF_ID;
//
//    extern std::vector<int> readInput_REAL_DATA_ID;
//    extern std::vector<int> readInput_NEW_LEAF_ID;
//    extern std::vector<int> readInput_BRANCH;
//    extern std::vector<int> readOutput_BRANCH;
//    extern std::vector<int> readOutput_DATA;
////    extern std::vector<int> readOutput_DATA_INSTRUCTION;
////    extern std::vector<int> readOutput_DATA_ARG1;
////    extern std::vector<int> readOutput_DATA_ARG2;
//    
//    extern std::vector<int> writeInput_REAL_DATA_ID;
//    extern std::vector<int> writeInput_NEW_LEAF_ID;
//    extern std::vector<int> writeInput_BRANCH;
//    extern std::vector<int> writeInput_WDATA;
//    extern std::vector<int> writeOutput_BRANCH;
////    std::vector<int> writeOutput_DATA;
    
    void configure(int memoryLength, int memoryCellLength, int instructionLength);
}

#endif
