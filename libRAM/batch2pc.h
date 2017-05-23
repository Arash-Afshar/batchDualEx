#ifndef BATCH_H
#define BATCH_H

#include <array>
#include <fstream>
#include <map>

//#include "boost/filesystem.hpp"

#include "XorHomomorphicCommit.h"
#include "XorHomomorphicCoordinator.h"

#include "cryptoTools/Network/BtEndpoint.h"
#include "cryptoTools/Common/Log.h"


#include "../libBDX/Circuit/Circuit.h"
#include "../libBDX/DualEx/DualExActor.h"
#include "constants.h"

namespace batchRam
{
    /**
     * The interface between the dualBatch2PC and the Coordinator
     */
    class Batch2PC
    {
        
    public:

        Batch2PC(std::string circ_path, osuCrypto::Role role, xhCoordinator::XHCCoordinator &xhcCoordinator, std::string name, int id, int numExec, int bucketSize, int numOpened, int psiSecParam, int numConcurrentSetups, int numConcurrentEvals, int numThreadsPerEval);
        
        /**
         * Sets the which inputs are to be get directly (through OT or normal commitments) and which inputs are to be passed from previous executions (through XOR Homomorphic commitments)
         * 
         * @param bucketIdx
         * @param input lengths of each party. We assume that the portion of inputs that is directly passed from the party starts form the beginning, hence only a length instead of a set of wire indexes
         */
        void setDirectInputWires(int bucketIdx, int lengths[2]);
        
        /**
         * Based on setDirectInputWires, call OT or sends commitments
         * It also checks whether the rest of inputs are set
         * 
         * @param bucketIdx
         */
        void prepareInputs(int bucketIdx);
        
        void evaluate(osuCrypto::u64 bucketIdx, std::vector<osuCrypto::block> &garbledOutputs, std::vector<uint64_t> inputWireIndexes, std::vector<std::vector<osuCrypto::block>> garbledInputValue);
        
        /**
         * Parse the output wires starting from the first output wire, O, to the output wire O+length and parse the result as a single number.
         * 
         * @param bucketIdx
         * @param length
         * @return Output value as a number
         */
        int getWiresAsPlainNumber(int bucketIdx, int length);
        
        /**
         * 
         * @param srcBktIdx
         * @param srcWireIndexes
         * @return the XOR Homomorphic commitments from the circuits for which you are the garbler. Note the bucket index and the lack of circuit identifier.
         */
        
        std::vector<std::array<xhCoordinator::XorHomomorphicCommit, 2> > getXHCommitments(int bktIdx, std::vector<int> wireIndexes);
        
        /**
         * 
         * @param bktIdx
         * @param wireIndexes
         * @return the commitment on the point and permute bits for the circuits for which you are the garbler
         */
        std::vector<xhCoordinator::XorHomomorphicCommit> getXHPointNPermuteCommitments(int bktIdx, std::vector<int> wireIndexes);
        
        /**
         * 
         * @param bktIdx
         * @param wireIndexes
         * @return a vector of the following form (garbledValue, [XHCommitment_on_wire, XHCommitment_on_point_n_permute])
         */
        std::vector<std::pair<osuCrypto::block, std::array<xhCoordinator::XorHomomorphicCommit, 2> > >  getWireAndXHCommitments(int bktIdx, std::vector<int> wireIndexes);
        
        /**
         * Set the value of garbledInputs. These values are coming from a previous evaluation of a 2PC
         * @param bktIdx
         * @param wireIndexes
         * @param garbledInputs
         */
        void setXHCInputs(int bktIdx, std::vector<int> wireIndexes, std::vector<osuCrypto::block> garbledInputs);
        
        /**
         * To set inputs and prng
         */
        void initEvaluate();

        /**
         * 
         * @return the name of computation. Used by coordinator to access the appropriate 2PC computation
         */
        std::string getName();
        
        /**
         * Close network connections
         */
        void cleanup();
        
        Identity getBucketHeadId(int bucketIdx);
        
//        std::vector<uint64_t> getRelativeOutputWireIndexes();
//        std::vector<uint64_t> getRelativeInputWireIndexes();
        
        std::vector<uint64_t> getInputSegmentWireIndexes(std::string segment);
        std::vector<uint64_t> getOutputSegmentWireIndexes(std::string segment);

        void initSegmentMaps() {};
        
    protected:
        std::unordered_map<std::string, std::vector<uint64_t>> inputSegments;
        std::unordered_map<std::string, std::vector<uint64_t>> outputSegments;
        
    private:
        std::string name;
        int computationId;
        std::string circuitPath;
        osuCrypto::Timer timer;
	osuCrypto::Circuit cir;
        osuCrypto::Role mRole;
        osuCrypto::PRNG *prng;
        osuCrypto::DualExActor *actor;
        osuCrypto::BtIOService *ios;
        osuCrypto::BtEndpoint *netMgr;
        osuCrypto::BitVector input;
        osuCrypto::PRNG *prng2;
        std::string mName;
        bool evalInited = false;

        
        /**
         * Reads a circuit from file
         * 
         * @param cir is the circuit representation of the input circuit definition
         * @param file
         */
        void read_circuit(osuCrypto::Circuit &cir, std::string file);
        
        
    };
    
    
    //------------------------- implementation headers
    class UniversalInstruction2PC : public Batch2PC{
    private:
        std::string genFilePath(std::string prefix){
            return prefix + "/universalInstruction-" + std::to_string(ramConfig::N) + "-" + std::to_string(ramConfig::dataLength) + ".circ";
        }

    public:
        UniversalInstruction2PC(std::string circ_path_prefix, osuCrypto::Role role, xhCoordinator::XHCCoordinator& xhcCoordinator, int id, int numExec, int bucketSize, int numOpened, int psiSecParam, int numConcurrentSetups, int numConcurrentEvals, int numThreadsPerEval) : 
        Batch2PC(genFilePath(circ_path_prefix), role, xhcCoordinator, "UI", id, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval)
        {
            initSegmentMaps();
        }
        
        void
        initSegmentMaps()
        {
            // ------------------ input
            std::vector<uint64_t> instruction(ramConfig::instructionLength);
            for(int i = 0; i < ramConfig::instructionLength; i++){
                instruction[i] = i;
            }
            std::vector<uint64_t> arg1(ramConfig::logN);
            std::vector<uint64_t> arg2(ramConfig::logN);
            for(int i = 0; i < ramConfig::logN; i++){
                arg1[i] = i + ramConfig::instructionLength;
                arg2[i] = i + ramConfig::instructionLength + ramConfig::logN;
            }
            std::vector<uint64_t> arg_data1(ramConfig::dataLength);
            std::vector<uint64_t> arg_data2(ramConfig::dataLength);
            for(int i = 0; i < ramConfig::dataLength; i++){
                arg_data1[i] = i + ramConfig::instructionLength + 2 * ramConfig::logN;
                arg_data2[i] = i + ramConfig::instructionLength + 2 * ramConfig::logN + ramConfig::dataLength;
            }    
            std::vector<uint64_t> pc(ramConfig::logN);
            for(int i = 0; i < ramConfig::logN; i++){
                pc[i] = i + ramConfig::instructionLength + 2 * ramConfig::logN + 2 * ramConfig::dataLength;
            }
            std::vector<uint64_t> rdata(ramConfig::dataLength);
            for(int i = 0; i < ramConfig::dataLength; i++){
                rdata[i] = i + ramConfig::instructionLength + 2 * ramConfig::logN + 2 * ramConfig::dataLength + ramConfig::logN;
            }
            
            inputSegments["INST"] = instruction;
            inputSegments["ARG1"] = arg1;
            inputSegments["ARG2"] = arg2;
            inputSegments["ARG_DATA1"] = arg_data1;
            inputSegments["ARG_DATA2"] = arg_data2;
            inputSegments["PC"] = pc;
            inputSegments["RDATA"] = pc;
            // ------------------ output

            int ui_output_start = ramConfig::instructionLength + 2 * ramConfig::logN + 2 * ramConfig::dataLength + ramConfig::logN + ramConfig::dataLength;
            std::vector<uint64_t> out_pc(ramConfig::logN);
            for(int i = 0; i < ramConfig::logN; i++){
                out_pc[i] = ui_output_start + i;
            }
            std::vector<uint64_t> out_rdata(ramConfig::dataLength);
            for(int i = 0; i < ramConfig::dataLength; i++){
                out_rdata[i] = ui_output_start + i + ramConfig::logN;
            }
            std::vector<uint64_t> out_wdata(ramConfig::dataLength);
            for(int i = 0; i < ramConfig::dataLength; i++){
                out_wdata[i] = ui_output_start + i + ramConfig::logN + ramConfig::dataLength;
            }
            std::vector<uint64_t> out_wdata_addr(ramConfig::logN);
            for(int i = 0; i < ramConfig::logN; i++){
                out_wdata_addr[i] = ui_output_start + i + ramConfig::logN + ramConfig::dataLength + ramConfig::dataLength;
            }
            
            outputSegments["PC"] = out_pc;
            outputSegments["RDATA"] = out_rdata;
            outputSegments["WDATA"] = out_wdata;
            outputSegments["WDATA_ADDR"] = out_wdata_addr;            
        }
        
    };

    class Read2PC : public Batch2PC{
    private:
        int recursiveDataLength;
        std::string genFilePath(std::string prefix, int dataLength){
            std::string path;
            if (dataLength <= 0){
                path = prefix + "/read-" + std::to_string(ramConfig::N) + "-" + std::to_string(ramConfig::dataLength) + ".circ";
            } else {
                path = prefix + "/read-" + std::to_string(ramConfig::N) + "-" + std::to_string(dataLength) + ".circ";
            }
            return path;
        }

    public:
        Read2PC(int dataLength, std::string circ_path_prefix, osuCrypto::Role role, xhCoordinator::XHCCoordinator& xhcCoordinator, int id, int numExec, int bucketSize, int numOpened, int psiSecParam, int numConcurrentSetups, int numConcurrentEvals, int numThreadsPerEval) : 
        Batch2PC(genFilePath(circ_path_prefix, dataLength), role, xhcCoordinator, "read", id, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval)
        {
            if (dataLength <= 0){
                recursiveDataLength = ramConfig::dataLength;
            } else {
                recursiveDataLength = dataLength;
            }
            initSegmentMaps();
        }
        
        void
        initSegmentMaps()
        {
            // ------------------ input
            int branch_length = ramConfig::elementsPerNode * (2 * ramConfig::logN + recursiveDataLength + 1) * (ramConfig::logN + 1);
            std::vector<uint64_t> branch(branch_length);
            for(int i = 0; i < branch_length; i++){
                branch[i] = i;
            }
            std::vector<uint64_t> real_addr(ramConfig::logN);
            for(int i = 0; i < ramConfig::logN; i++){
                real_addr[i] = i + branch_length;
            }
            std::vector<uint64_t> leaf_id(ramConfig::logN);
            for(int i = 0; i < ramConfig::logN; i++){
                leaf_id[i] = i + branch_length + ramConfig::logN;
            }
//            std::vector<uint64_t> wdata(ramConfig::dataLength);
//            for(int i = 0; i < ramConfig::dataLength; i++){
//                wdata[i] = i + branch_length + ramConfig::logN + ramConfig::logN;
//            }
            inputSegments["BRANCH"] = branch;
            inputSegments["REAL_ID"] = real_addr;
            inputSegments["LEAF_ID"] = leaf_id;
//            inputSegments["WDATA"] = wdata;

            // ------------------- output
            std::vector<uint64_t> out_branch(branch_length);
            for(int i = 0; i < branch_length; i++){
                out_branch[i] = i + branch_length + ramConfig::logN + ramConfig::logN;
//                out_branch[i] = i + branch_length + ramConfig::logN + ramConfig::logN + recursiveDataLength;
            }
            std::vector<uint64_t> out_rdata(recursiveDataLength);
            for(int i = 0; i < recursiveDataLength; i++){
                out_rdata[i] = i + branch_length + ramConfig::logN + ramConfig::logN + branch_length;
            }
            outputSegments["BRANCH"] = out_branch;
            outputSegments["RDATA"] = out_rdata;            
        }
        
    };

    class Write2PC : public Batch2PC{
    private:
        int recursiveDataLength;
        std::string genFilePath(std::string prefix, int dataLength){
            std::string path;
            if (dataLength <= 0){
                path = prefix + "/write-" + std::to_string(ramConfig::N) + "-" + std::to_string(ramConfig::dataLength) + ".circ";
            } else {
                path = prefix + "/write-" + std::to_string(ramConfig::N) + "-" + std::to_string(dataLength) + ".circ";
            }
            return path;
        }

    public:
        Write2PC(int dataLength, std::string circ_path_prefix, osuCrypto::Role role, xhCoordinator::XHCCoordinator& xhcCoordinator, int id, int numExec, int bucketSize, int numOpened, int psiSecParam, int numConcurrentSetups, int numConcurrentEvals, int numThreadsPerEval) : 
        Batch2PC(genFilePath(circ_path_prefix, dataLength), role, xhcCoordinator, "write", id, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval)
        {
            if (dataLength <= 0){
                recursiveDataLength = ramConfig::dataLength;
            } else {
                recursiveDataLength = dataLength;
            }
            initSegmentMaps();
        }
        
        void
        initSegmentMaps()
        {
            // ------------------ input
            int branch_length = ramConfig::elementsPerNode * (2 * ramConfig::logN + recursiveDataLength + 1) * (ramConfig::logN + 1);
            std::vector<uint64_t> branch(branch_length);
            for(int i = 0; i < branch_length; i++){
                branch[i] = i;
            }
            std::vector<uint64_t> real_addr(ramConfig::logN);
            for(int i = 0; i < ramConfig::logN; i++){
                real_addr[i] = i + branch_length;
            }
            std::vector<uint64_t> leaf_id(ramConfig::logN);
            for(int i = 0; i < ramConfig::logN; i++){
                leaf_id[i] = i + branch_length + ramConfig::logN;
            }
            std::vector<uint64_t> wdata(ramConfig::dataLength);
            for(int i = 0; i < ramConfig::dataLength; i++){
                wdata[i] = i + branch_length + ramConfig::logN + ramConfig::logN;
            }
            inputSegments["BRANCH"] = branch;
            inputSegments["REAL_ID"] = real_addr;
            inputSegments["LEAF_ID"] = leaf_id;
            inputSegments["WDATA"] = wdata;

            // ------------------- output
            std::vector<uint64_t> out_branch(branch_length);
            for(int i = 0; i < branch_length; i++){
                out_branch[i] = i + branch_length + ramConfig::logN + ramConfig::logN + recursiveDataLength;
            }
            outputSegments["BRANCH"] = out_branch;
        }
        
    };


    class Evict2PC : public Batch2PC{
    private:
        int recursiveDataLength;
        std::string genFilePath(std::string prefix, int dataLength){
            std::string path;
            if (dataLength <= 0){
                path = prefix + "/evict-" + std::to_string(ramConfig::N) + "-" + std::to_string(ramConfig::dataLength) + ".circ";
            } else {
                path = prefix + "/evict-" + std::to_string(ramConfig::N) + "-" + std::to_string(dataLength) + ".circ";
            }
            return path;
        }
        
    public:
        Evict2PC(int dataLength, std::string circ_path_prefix, osuCrypto::Role role, xhCoordinator::XHCCoordinator& xhcCoordinator, int id, int numExec, int bucketSize, int numOpened, int psiSecParam, int numConcurrentSetups, int numConcurrentEvals, int numThreadsPerEval) : 
        Batch2PC(genFilePath(circ_path_prefix, dataLength), role, xhcCoordinator, "evict", id, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval)
        {
            if (dataLength <= 0){
                recursiveDataLength = ramConfig::dataLength;
            } else {
                recursiveDataLength = dataLength;
            }
            initSegmentMaps();
        }
        
        void
        initSegmentMaps()
        {
            // ------------------ input
            int branch_length = ramConfig::elementsPerNode * (2 * ramConfig::logN + recursiveDataLength + 1) * (ramConfig::logN + 1);
            std::vector<uint64_t> branch(branch_length);
            for(int i = 0; i < branch_length; i++){
                branch[i] = i;
            }
            std::vector<uint64_t> real_addr(ramConfig::logN);
            for(int i = 0; i < ramConfig::logN; i++){
                real_addr[i] = i + branch_length;
            }
            inputSegments["BRANCH"] = branch;
            inputSegments["REAL_ID"] = real_addr;
            
            // ------------------ output
            std::vector<uint64_t> out_branch(branch_length);
            for(int i = 0; i < branch_length; i++){
                out_branch[i] = i + branch_length + ramConfig::logN;
            }
            outputSegments["BRANCH"] = out_branch;
        }
        
    };

    
    
}

#endif