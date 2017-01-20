
#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <vector>
#include <cassert>
#include <string>

#include "batch2pc.h"
#include "ram.h"
#include "XorHomomorphicCoordinator.h"
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/ByteStream.h"
#include "cryptoTools/Network/Channel.h"

namespace batchRam
{
    class Coordinator
    {
    public:
        
        Coordinator(osuCrypto::Channel &chl, osuCrypto::Role role, std::string circ_path_prefix, int memoryLength, int memoryCellLength, int instructionLength, int numExec, int bucketSize, int numOpened, int psiSecParam, int numConcurrentSetups, int numConcurrentEvals, int numThreadsPerEval);
        
        /**
         * Closes channels and takes care of housekeeping
         * @return 
         */
        void cleanup();
        
        /**
         * Initialize channel, ram, and other stuff!!
         */
        void initialize(std::string circ_path_prefix, xhCoordinator::XHCCoordinator xhcCoordinator, int numExec, int bucketSize, int numOpened, int psiSecParam, int numConcurrentSetups, int numConcurrentEvals, int numThreadsPerEval);
        
        void mainLoop();
        
        /**
         * The first time the lookup is called, it needs to be passed the initial lookup table and the initial PC value
         * Apart from this, it behaves like lookup function
         */
        
        void initialLookup();
        /**
         * Lookup searches the garbled lookup table for a real memory id and outputs the matching plain leaf id.
         * It also gets two random values and from them, computes the new leaf id of the relocated data.
         * Moreover, it updates the lookup table and outputs the updated garbled lookup table
         * 
         * @param bucketIdx
         * @param src
         * @param srcBktIdx
         * @param srcInputWireIndexes
         */
        void lookup(int bucketIdx, Batch2PC src, int srcBktIdx, std::vector<int> srcInputWireIndexes);
        
        /**
         * It reads the branch corresponding to the previously generated leaf id (from lookup).
         * After removing the data and adding it to the root with new leaf id, it calls evict
         * 
         * @param readBucketIdx
         * @param lookupBktIdx
         */
        void readMem(int readBucketIdx, int lookupBktIdx);
        
        /**
         * It does everything readMem does + it updates the memory value
         * 
         * @param readBucketIdx
         * @param lookupBktIdx
         */        
        void writeMem(int writeBktIdx, int lookupBktIdx);
        
        /**
         * Evict always starts at root of the memory tree and pushes the data down the nodes
         */
        void evict();
        
        /**
         * It connects a set of garbled output values from a previous execution to the garbled inputs needed for current execution.
         * In the process, it needs to check the correctness of the XOR homomorphic commitments
         * 
         * @param src
         * @param srcBktIdx
         * @param srcWireIndexes
         * @param dst
         * @param dstBktIdx
         * @param dstWireIndexes
         */
        void connectBktToBkt(Batch2PC src, int srcBktIdx, std::vector<int> srcWireIndexes, Batch2PC dst, int dstBktIdx, std::vector<int> dstWireIndexes);

        /**
         * It connects a set of garbled output values from a set of previous execution (which are stored in the RAM) to the garbled inputs needed for current execution.
         * In the process, it needs to check the correctness of the XOR homomorphic commitments
         * 
         * @param src
         * @param srcBktIdx
         * @param srcWireIndexes
         */
        void connectRamToBkt(Batch2PC src, int srcBktIdx, std::vector<int> srcWireIndexes);
        
        /**
         * It fetches the argument values for the current instruction
         * 
         * @param t
         */
        void readArgs(int t);
        
        /**
         * Checks whether the current instruction is halt. In case of a halt, the program should keep track of the reported output but keep running until the pre-specified iteration counts
         * 
         * @param bucketIdx
         * @return 
         */
        bool isHalt(int bucketIdx);
        
        /**
         * Behaves the same way as universalInstruction, except that the input values for PC and last read data are set to zero.
         */
        void initialUniversalInstruction();

        /**
         * The universal instruction executes an instruction based on the last read value, the instruction itself and its arguments.
         * It then updates the last read data and the program counter (PC).
         * It also instructs a position in memory that needs to be updated.
         */
        void universalInstruction(int bucketIdx);
        
        
    private:
        Batch2PC *lookup2PC;
        Batch2PC *read2PC;
        Batch2PC *write2PC;
        Batch2PC *halt2PC;
        Batch2PC *universalInstruction2PC;
//        RandomAccessMemory ram;
        int executionTime;
        int nextLeafAccess;
        int prevLookupIdx;
        osuCrypto::Channel &chl;
        osuCrypto::Role role;
        xhCoordinator::XHCCoordinator *xhcCoordinator;
        
        /**
         * The common part between initialUniversalInstruction and universalInstruction which takes care of setting the inputs
         * 
         * @param bucketIdx
         */
        void commonUniversalInstructionSetup(int bucketIdx);

        /**
         * 
         * Converts the garbled output values to garbled input values
         * 
         * @param deltas
         * @param srcWireAndCommitments
         * @return 
         */
        std::vector<osuCrypto::block> convert(std::vector<std::array<osuCrypto::block, 2> > deltas, std::vector<std::pair<osuCrypto::block, std::array<xhCoordinator::XorHomomorphicCommit, 2> > >  srcWireAndCommitments);
    };

}

#endif