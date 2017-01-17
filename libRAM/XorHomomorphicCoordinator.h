
#ifndef XHC_H
#define XHC_H

#include <array>
#include "cryptoTools/Common/Defines.h"
#include "XorHomomorphicCommit.h"
#include "identity.h"


namespace xhCoordinator
{
 
    class XHCCoordinator
    {
    public:
        /**
         * Receives garbled input keys (both 0-key and 1-key) and the permutation bit, stores them, commits to them, and send them to the other party
         * 
         * @param permBit
         * @param allInputLabels
         * @param id: identity of the bucket who has sent these values
         */
        void commitToInput(std::vector<bool> permBit, std::vector<std::array<osuCrypto::block, 2> > allInputLabels, Identity id);
        
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
        void receiveInputCommitments(Identity id);
        
        /**
         * Receives the output commitments
         * @param id
         */
        void receiveOutputCommitments(Identity id);
        
    };
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