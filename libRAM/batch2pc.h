#ifndef BATCH_H
#define BATCH_H

#include <array>
#include <fstream>

//#include "boost/filesystem.hpp"

#include "XorHomomorphicCommit.h"
#include "XorHomomorphicCoordinator.h"

#include "cryptoTools/Network/BtEndpoint.h"
#include "cryptoTools/Common/Log.h"


#include "../libBDX/Circuit/Circuit.h"
#include "../libBDX/DualEx/DualExActor.h"

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
        
        void evaluate(osuCrypto::u64 bucketIdx, std::vector<osuCrypto::block> &garbledOutputs);
        
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
        
        std::vector<int> getRelativeOutputWireIndexes();
        std::vector<int> getRelativeInputWireIndexes();
        
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
        bool evalInited = false;

        
        /**
         * Reads a circuit from file
         * 
         * @param cir is the circuit representation of the input circuit definition
         * @param file
         */
        void read_circuit(osuCrypto::Circuit &cir, std::string file);
        
    };
}

#endif