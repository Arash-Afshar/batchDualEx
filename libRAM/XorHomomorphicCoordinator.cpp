#include "XorHomomorphicCoordinator.h"

namespace xhCoordinator
{
 
    void
    XHCCoordinator::commitToInput(std::vector<bool> permBit, std::vector<std::array<osuCrypto::block, 2> > allInputLabels, Identity id)
    {
        
    }
    
    void
    XHCCoordinator::receiveInputCommitments(Identity id)
    {
        
    }
    
    void
    XHCCoordinator::commitToOutput(std::vector<bool> permBit, std::vector<std::array<osuCrypto::block, 2> > allOutputLabels, Identity id)
    {
        
    }
    
    void
    XHCCoordinator::receiveOutputCommitments(Identity id)
    {
        
    }
    
    osuCrypto::block
    computeDelta(xhCoordinator::XorHomomorphicCommit srcXHCommitment, xhCoordinator::XorHomomorphicCommit dstXHCommitment)
    {
        
    }

    std::vector<std::array<osuCrypto::block, 2> >
    computeDeltas(std::vector<std::array<xhCoordinator::XorHomomorphicCommit, 2> > srcXHCommitments, std::vector<std::array<xhCoordinator::XorHomomorphicCommit, 2> > dstXHCommitments)
    {
        
    }

    std::vector<osuCrypto::block>
    computeDeltas(std::vector<xhCoordinator::XorHomomorphicCommit> srcXHCommitments, std::vector<xhCoordinator::XorHomomorphicCommit> dstXHCommitments)
    {
        
    }

    bool
    verify(std::vector<std::array<osuCrypto::block, 2> > deltas, std::vector<osuCrypto::block> pointNPermuteDeltas, std::vector<std::pair<osuCrypto::block, std::array<xhCoordinator::XorHomomorphicCommit, 2> > >  srcWireAndCommitments, std::vector<std::pair<osuCrypto::block, std::array<xhCoordinator::XorHomomorphicCommit, 2> > > dstWireAndCommitments)
    {
        
    }
}