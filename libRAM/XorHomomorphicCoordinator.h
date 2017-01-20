
#ifndef XHC_H
#define XHC_H

#include <array>
#include "cryptoTools/Common/Defines.h"
#include "XorHomomorphicCommit.h"
#include "identity.h"
#include "split-commit/split-commit-snd.h"
#include "split-commit/split-commit-rec.h"


namespace xhCoordinator
{
 
    class XHCCoordinator
    {
    public:
        
        XHCCoordinator(int num_commits, osuCrypto::Role role);
        
        /**
         * Receives garbled input keys (both 0-key and 1-key) and the permutation bit, stores them, commits to them, and send them to the other party
         * 
         * @param permBit
         * @param allInputLabels
         * @param id: identity of the bucket who has sent these values
         */
        void commitToInput(std::vector<bool> permBit, std::vector<osuCrypto::block> allInputLabels, Identity id, osuCrypto::Role role, osuCrypto::Channel &send_channel);
        
        /**
         * Behaves exactly as commitToInput but for output wires
         * 
         * @param permBit
         * @param allOutputLabels
         * @param id
         */
        void commitToOutput(std::vector<bool> permBit, std::vector<std::array<osuCrypto::block, 2> > allOutputLabels, Identity id);
        
        /**
         * Receives the input commitments
         * 
         * @param id
         */
        void receiveInputCommitments(Identity id, osuCrypto::Role role, int inputSize, osuCrypto::Channel &rec_channel);
        
        /**
         * Receives the output commitments
         * @param id
         */
        void receiveOutputCommitments(Identity id);
        
    private:
        ctpl::thread_pool *thread_pool;
        std::vector<BYTEArrayVector> rec_commit_shares;
        std::vector<std::array<BYTEArrayVector, 2> > send_commit_shares;
        osuCrypto::BtEndpoint *end_point;

        void randomSenderCommits(std::vector<SplitCommitSender> &senders, osuCrypto::Role role, int num_execs, int num_commits);
        void randomReceiverCommits(std::vector<SplitCommitReceiver> &receivers, std::vector<osuCrypto::PRNG> &exec_rnds, osuCrypto::Role role, int num_execs, int num_commits);

    }; 
    
    
    void xorUI8s(std::vector<uint8_t> &ret, int pos, uint8_t *vec1, int vec1_size, uint8_t *vec2, int vec2_size);


    
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