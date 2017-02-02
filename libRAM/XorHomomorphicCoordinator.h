
#ifndef XHC_H
#define XHC_H

#include <array>
#include <mutex>

#include "cryptoTools/Common/Defines.h"
#include "componentConfig.h"
#include "XorHomomorphicCommit.h"
#include "identity.h"
#include "split-commit/split-commit-snd.h"
#include "split-commit/split-commit-rec.h"

namespace xhCoordinator
{
 
    class XHCCoordinator
    {
    public:
        
        XHCCoordinator() = delete;
        XHCCoordinator(int num_commits, osuCrypto::Role role);
        ~XHCCoordinator();

        
        /**
         * Receives garbled input keys (both 0-key and 1-key) and the permutation bit, stores them, commits to them, and send them to the other party
         * 
         * @param permBit
         * @param allInputLabels
         * @param id: identity of the bucket who has sent these values
         */
        void commitToInput(std::vector<bool> permBit, std::vector<osuCrypto::block> allInputLabels, Identity id, osuCrypto::Channel &send_channel);
        
        /**
         * Behaves exactly as commitToInput but for output wires
         * 
         * @param permBit
         * @param allOutputLabels
         * @param id
         */
        void commitToOutput(std::vector<bool> permBit, std::vector<osuCrypto::block> allOutputLabels, Identity id, osuCrypto::Channel &send_channel);

        /**
         * Receives the input commitments
         * @param id
         * @param inputSize
         * @param rec_channel
         */
        void receiveInputCommitments(Identity id, int inputSize, osuCrypto::Channel &rec_channel);
        
        /**
         * Receives the output commitments
         * @param id
         * @param outputSize
         * @param rec_channel
         */
        void receiveOutputCommitments(Identity id, int outputSize, osuCrypto::Channel &rec_channel);
        
        void translateBucketHeads(Identity srcId, std::vector<int> outputWireIndexes, std::vector<osuCrypto::block> garbledOutputValue, Identity dstId, std::vector<int> inputWireIndexes, std::vector<osuCrypto::block> &garbledInputValue, uint64_t evalId);
        
    private:
        std::vector<SplitCommitSender> *senders;
        std::vector<SplitCommitReceiver> *receivers;
        ctpl::thread_pool *thread_pool;
        std::mutex mtx;
        std::vector<BYTEArrayVector> rec_commit_shares;
        std::vector<std::array<BYTEArrayVector, 2> > send_commit_shares;
        osuCrypto::BtEndpoint *end_point;
        osuCrypto::BtIOService *ios;
        osuCrypto::Role mRole;
        std::vector<std::vector<std::vector<uint8_t>>> actualOutputCommitments;
        std::vector<std::vector<std::vector<uint8_t>>> actualInputCommitments;
//        std::array<BYTEArrayVector, 2> random_commitments;
        // these are hard coded configs
        // TODO: should be configured in config.cpp and accessed here
        int startInRandCommit[4];
        int perCircuitSize[4];
        int perInputSize[4];
        int outputStartOffset[4];

        void sendRandomCommits(std::vector<SplitCommitSender> &senders, osuCrypto::Role role, int num_execs, int num_commits);
        void receiveRandomCommits(std::vector<SplitCommitReceiver> &receivers, std::vector<osuCrypto::PRNG> &exec_rnds, osuCrypto::Role role, int num_execs, int num_commits);
        /**
         * The underlying function for both commitToInput and CommitToOutput
         * 
         * @param permBit
         * @param allLabels
         * @param id
         * @param send_channel
         * @param isInput
         */
        void commitToIO(std::vector<bool> permBit, std::vector<osuCrypto::block> allLabels, Identity id, osuCrypto::Channel &send_channel, bool isInput, std::vector<uint8_t> &actualIOCommitments);

        /**
         * Returns the random shares needed for committing to 0-key and 1-key and also the permutation bit. Therefore, output is an array of length 3 * 2 = 6
         * 
         * @param id
         * @param wireIndx
         * @param isInput
         * @param output
         */
        void getRandomCommitment(Identity id, int wireIndx, bool isInput, std::array<uint8_t *, 6> &output);
        
        void getPosInCommitShares(Identity id, int startOffset, int wireIndx, bool isInput, int &exec, int &offset);
        
    }; 
    
    
//    void xorUI8s(std::vector<uint8_t> &ret, int pos, uint8_t *vec1, int vec1_size, uint8_t *vec2, int vec2_size);
//    void xorUI8s(std::vector<uint8_t> &ret, int pos, int length, uint8_t *vec2, int vec2_size);
    void xorBlockWithUint8(osuCrypto::block &dst, osuCrypto::block src, uint8_t *vec2);
    void print(std::string desc, uint8_t *vec, int vec_num_entries);
    std::string chlIdStr(std::string name, osuCrypto::Role role, bool isSender, bool isFrom);


    
    // TODO: move the following to the above class
    
//        
//    public:
        /**
         * gets too commitments and sends the opening to their XOR
         * 
         * TODO: it probably need to get bucket ID to access its internal state and find the decommitments...
         * 
         * @param srcXHCommitment
         * @param dstXHCommitment
         * @return 
         */
        osuCrypto::block computeDelta(XorHomomorphicCommit srcXHCommitment, XorHomomorphicCommit dstXHCommitment);
        
        /**
         * Uses computeDelta to compute the opening for the commitments on garbled keys
         * 
         * @param srcXHCommitments
         * @param dstXHCommitments
         * @return 
         */
        std::vector<std::array<osuCrypto::block, 2> > computeDeltas(std::vector<std::array<XorHomomorphicCommit, 2> > srcXHCommitments, std::vector<std::array<XorHomomorphicCommit, 2> > dstXHCommitments);
        
        /**
         * Uses computeDelta to compute the opening for the commitments on the point-and-permute bits
         * 
         * @param srcXHCommitments
         * @param dstXHCommitments
         * @return 
         */
        std::vector<osuCrypto::block> computeDeltas(std::vector<XorHomomorphicCommit> srcXHCommitments, std::vector<XorHomomorphicCommit> dstXHCommitments);
        
        /**
         * Verifies that
         *      1) the opening of the garbled values are correct
         *      2) the opening of the point-n-permutes are correct
         *      3) the opened point-n-permute matches the XOR of the position bit of the opened garbled key
         * 
         * @param deltas
         * @param pointNPermuteDeltas
         * @param srcWireAndCommitments
         * @param dstWireAndCommitments
         * @return 
         */
        bool verify(std::vector<std::array<osuCrypto::block, 2> > deltas, std::vector<osuCrypto::block> pointNPermuteDeltas, std::vector<std::pair<osuCrypto::block, std::array<XorHomomorphicCommit, 2> > >  srcWireAndCommitments, std::vector<std::pair<osuCrypto::block, std::array<XorHomomorphicCommit, 2> > > dstWireAndCommitments);

//    };
}

#endif