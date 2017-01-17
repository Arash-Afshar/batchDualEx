
#include "constants.h"

int ramConfig::N;
int ramConfig::logN;
int ramConfig::argLength;
int ramConfig::leafIdLength;
int ramConfig::dataLength;
int ramConfig::elementsPerNode;

std::vector<int> ramConfig::universalInstructionInput_INSTRUCTION;
std::vector<int> ramConfig::universalInstructionInput_ARG1;
std::vector<int> ramConfig::universalInstructionInput_ARG2;
std::vector<int> ramConfig::universalInstructionInput_PC;
std::vector<int> ramConfig::universalInstructionInput_RDATA;
std::vector<int> ramConfig::universalInstructionOutput_PC;
std::vector<int> ramConfig::universalInstructionOutput_RDATA;
std::vector<int> ramConfig::universalInstructionOutput_WDATA;
std::vector<int> ramConfig::universalInstructionOutput_WDATA_ADDR;

std::vector<int> ramConfig::lookupInput_REAL_DATA_ID;
std::vector<int> ramConfig::lookupInput_TABLE;
std::vector<int> ramConfig::lookupOutput_REAL_DATA_ID;
std::vector<int> ramConfig::lookupOutput_TABLE;
std::vector<int> ramConfig::lookupOutput_NEW_LEAF_ID;

std::vector<int> ramConfig::readInput_REAL_DATA_ID;
std::vector<int> ramConfig::readInput_NEW_LEAF_ID;
std::vector<int> ramConfig::readInput_BRANCH;
std::vector<int> ramConfig::readOutput_BRANCH;
std::vector<int> ramConfig::readOutput_DATA;
std::vector<int> ramConfig::readOutput_DATA_INSTRUCTION;
std::vector<int> ramConfig::readOutput_DATA_ARG1;
std::vector<int> ramConfig::readOutput_DATA_ARG2;

std::vector<int> ramConfig::writeInput_REAL_DATA_ID;
std::vector<int> ramConfig::writeInput_NEW_LEAF_ID;
std::vector<int> ramConfig::writeInput_BRANCH;
std::vector<int> ramConfig::writeInput_WDATA;
std::vector<int> ramConfig::writeOutput_BRANCH;

void
ramConfig::configure(int memoryLength, int memoryCellLength, int instructionLength)
{
    // TODO: from these parameters, initialize the input wire indexes that the coordinator will use
    ramConfig::N = memoryLength;
    ramConfig::logN = std::log(N)/std::log(2);
    ramConfig::argLength = logN;
    ramConfig::leafIdLength = logN;
    ramConfig::dataLength = memoryCellLength;
}
