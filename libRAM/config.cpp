
#include "constants.h"
 
int ramConfig::instructionLength;
int ramConfig::N;
int ramConfig::logN;
//int ramConfig::argLength;
int ramConfig::leafIdLength;
int ramConfig::dataLength;
int ramConfig::elementsPerNode;

//std::vector<int> ramConfig::universalInstructionInput_INSTRUCTION;
//std::vector<int> ramConfig::universalInstructionInput_ARG1;
//std::vector<int> ramConfig::universalInstructionInput_ARG2;
//std::vector<int> ramConfig::universalInstructionInput_ARG_DATA1;
//std::vector<int> ramConfig::universalInstructionInput_ARG_DATA2;
//std::vector<int> ramConfig::universalInstructionInput_PC;
//std::vector<int> ramConfig::universalInstructionInput_RDATA;
//std::vector<int> ramConfig::universalInstructionOutput_PC;
//std::vector<int> ramConfig::universalInstructionOutput_RDATA;
//std::vector<int> ramConfig::universalInstructionOutput_WDATA;
//std::vector<int> ramConfig::universalInstructionOutput_WDATA_ADDR;
//
//std::vector<int> ramConfig::lookupInput_REAL_DATA_ID;
//std::vector<int> ramConfig::lookupInput_TABLE;
//std::vector<int> ramConfig::lookupOutput_REAL_DATA_ID;
//std::vector<int> ramConfig::lookupOutput_TABLE;
//std::vector<int> ramConfig::lookupOutput_NEW_LEAF_ID;
//
//std::vector<int> ramConfig::readInput_BRANCH;
//std::vector<int> ramConfig::readInput_REAL_DATA_ID;
//std::vector<int> ramConfig::readInput_NEW_LEAF_ID;
//std::vector<int> ramConfig::readOutput_BRANCH;
//std::vector<int> ramConfig::readOutput_DATA;
////std::vector<int> ramConfig::readOutput_DATA_INSTRUCTION;
////std::vector<int> ramConfig::readOutput_DATA_ARG1;
////std::vector<int> ramConfig::readOutput_DATA_ARG2;
//
//std::vector<int> ramConfig::writeInput_BRANCH;
//std::vector<int> ramConfig::writeInput_REAL_DATA_ID;
//std::vector<int> ramConfig::writeInput_NEW_LEAF_ID;
//std::vector<int> ramConfig::writeInput_WDATA;
//std::vector<int> ramConfig::writeOutput_BRANCH;
//
void
ramConfig::configure(int memoryLength, int memoryCellLength, int instructionLength)
{
    // TODO: from these parameters, initialize the input wire indexes that the coordinator will use
    ramConfig::elementsPerNode = 4;
    ramConfig::instructionLength = instructionLength;
    ramConfig::N = memoryLength;
    ramConfig::logN = std::log(N)/std::log(2);
//    ramConfig::argLength = logN;
    ramConfig::leafIdLength = logN;
    ramConfig::dataLength = memoryCellLength;
    
    
//    // ---------------- UI wires -----------------------------------------------
//    // input
//    ramConfig::universalInstructionInput_INSTRUCTION.resize(4);
//    for(int i = 0; i < 4; i++){
//        ramConfig::universalInstructionInput_INSTRUCTION[i] = i;
//    }
//    ramConfig::universalInstructionInput_ARG1.resize(ramConfig::logN);
//    ramConfig::universalInstructionInput_ARG2.resize(ramConfig::logN);
//    for(int i = 0; i < ramConfig::logN; i++){
//        ramConfig::universalInstructionInput_ARG1[i] = i + 4;
//        ramConfig::universalInstructionInput_ARG2[i] = i + 4 + ramConfig::logN;
//    }
//    ramConfig::universalInstructionInput_ARG_DATA1.resize(ramConfig::dataLength);
//    ramConfig::universalInstructionInput_ARG_DATA2.resize(ramConfig::dataLength);
//    for(int i = 0; i < ramConfig::dataLength; i++){
//        ramConfig::universalInstructionInput_ARG_DATA1[i] = i + 4 + 2 * ramConfig::logN;
//        ramConfig::universalInstructionInput_ARG_DATA2[i] = i + 4 + 2 * ramConfig::logN + ramConfig::dataLength;
//    }    
//    ramConfig::universalInstructionInput_PC.reserve(ramConfig::logN);
//    for(int i = 0; i < ramConfig::logN; i++){
//        ramConfig::universalInstructionInput_PC[i] = i + 4 + 2 * ramConfig::logN + 2 * ramConfig::dataLength;
//    }
//    ramConfig::universalInstructionInput_RDATA.resize(ramConfig::dataLength);
//    for(int i = 0; i < ramConfig::dataLength; i++){
//        ramConfig::universalInstructionInput_RDATA[i] = i + 4 + 2 * ramConfig::logN + 2 * ramConfig::dataLength + ramConfig::logN;
//    }
//    // output
//    int ui_output_start = 4 + 2 * ramConfig::logN + 2 * ramConfig::dataLength + ramConfig::logN + ramConfig::dataLength;
//    ramConfig::universalInstructionOutput_PC.resize(ramConfig::logN);
//    for(int i = 0; i < ramConfig::logN; i++){
//        ramConfig::universalInstructionOutput_PC[i] = ui_output_start + i;
//    }
//    ramConfig::universalInstructionOutput_RDATA.resize(ramConfig::dataLength);
//    for(int i = 0; i < ramConfig::dataLength; i++){
//        ramConfig::universalInstructionOutput_RDATA[i] = ui_output_start + i + ramConfig::logN;
//    }
//    ramConfig::universalInstructionOutput_WDATA.resize(ramConfig::dataLength);
//    for(int i = 0; i < ramConfig::dataLength; i++){
//        ramConfig::universalInstructionOutput_WDATA[i] = ui_output_start + i + ramConfig::logN + ramConfig::dataLength;
//    }
//    ramConfig::universalInstructionOutput_WDATA_ADDR.resize(ramConfig::logN);
//    for(int i = 0; i < ramConfig::logN; i++){
//        ramConfig::universalInstructionOutput_WDATA_ADDR[i] = ui_output_start + i + ramConfig::logN + ramConfig::dataLength + ramConfig::dataLength;
//    }
//
//    // ---------------- read/write wires -----------------------------------------------
//    // input
//    int branch_length = ramConfig::elementsPerNode * (2 * ramConfig::logN + ramConfig::dataLength + 1) * (ramConfig::logN + 1);
//    ramConfig::readInput_BRANCH.resize(branch_length);
//    ramConfig::writeInput_BRANCH.resize(branch_length);
//    for(int i = 0; i < branch_length; i++){
//        ramConfig::readInput_BRANCH[i] = i;
//        ramConfig::writeInput_BRANCH[i] = i;
//    }
//    ramConfig::readInput_REAL_DATA_ID.resize(ramConfig::logN);
//    ramConfig::writeInput_REAL_DATA_ID.resize(ramConfig::logN);
//    for(int i = 0; i < ramConfig::logN; i++){
//        ramConfig::readInput_REAL_DATA_ID[i] = i + branch_length;
//        ramConfig::writeInput_REAL_DATA_ID[i] = i + branch_length;
//    }
//    ramConfig::readInput_NEW_LEAF_ID.resize(ramConfig::logN);
//    ramConfig::writeInput_NEW_LEAF_ID.resize(ramConfig::logN);
//    for(int i = 0; i < ramConfig::logN; i++){
//        ramConfig::readInput_NEW_LEAF_ID[i] = i + branch_length + ramConfig::logN;
//        ramConfig::writeInput_NEW_LEAF_ID[i] = i + branch_length + ramConfig::logN;
//    }
//    ramConfig::writeInput_WDATA.resize(ramConfig::dataLength);
//    for(int i = 0; i < ramConfig::dataLength; i++){
//        ramConfig::writeInput_WDATA[i] = i + branch_length + ramConfig::logN + ramConfig::logN;
//    }
//    
//    // output
//    ramConfig::readOutput_BRANCH.resize(branch_length);
//    ramConfig::writeOutput_BRANCH.resize(branch_length);
//    for(int i = 0; i < branch_length; i++){
//        ramConfig::readOutput_BRANCH[i] = i + branch_length + ramConfig::logN + ramConfig::logN;
//        ramConfig::writeOutput_BRANCH[i] = i + branch_length + ramConfig::logN + ramConfig::logN + ramConfig::dataLength;
//    }
//    ramConfig::readOutput_DATA.resize(ramConfig::dataLength);
//    for(int i = 0; i < ramConfig::dataLength; i++){
//        ramConfig::readOutput_DATA[i] = i + branch_length + ramConfig::logN + ramConfig::logN + branch_length;
//    }
//

}
