#ifndef TREE_H
#define TREE_H

#include <array>
#include <bitset>
#include "cryptoTools/Common/Defines.h"
#include "XorHomomorphicCommit.h"
#include "batch2pc.h"
#include "constants.h"

namespace batchRam
{
    
    //---------------------------------------------------------------------------------
    struct GarbledData
    {
        osuCrypto::block dummyIndicator;
        std::vector<osuCrypto::block> data;
        std::vector<osuCrypto::block> realID;
        std::vector<osuCrypto::block> leafID;
        std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit>  dummyIndicatorCommitment;
        std::vector<std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit> >  dataCommitment;
        std::vector<std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit> >  realIDCommitment;
        std::vector<std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit> >  leafIDCommitment;
    };
    
    //---------------------------------------------------------------------------------
    class Node
    {
    public:
        Node* children;
        int capacity;

        Node()
        {
            capacity = ramConfig::elementsPerNode;
        }        
        Node(std::vector<GarbledData> nodeData);
        
        void setGarbledNode(std::vector<GarbledData> nodeData);
        std::vector<GarbledData> getGarbledNode();
        std::vector<std::vector<std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit> > > getCommitments();
        std::vector<std::vector<osuCrypto::block> > getGarbledValues();
        
    private:
        std::vector<GarbledData> nodeData;
        
    };
    
    //---------------------------------------------------------------------------------

    class Branch
    {
    public:
        Branch(Node root, int leaf);
        std::pair<Node, int> getNodeAt(int i);
        std::vector<std::vector<std::vector<std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit> > > > getXHCommitments();
        
    private:
        int height;
        std::vector<Node> nodes;
        std::vector<bool> directions;
    };
    
    //---------------------------------------------------------------------------------

    class Tree
    {
    public:
        Node root;
        
        Tree();
        Branch getBranch(int leafId);
        void setBranch(int leafId, Branch branch);
        Node getNode(int level, int nodePos);
        
    private:
        int height;
        std::vector<std::vector<Node> > levels;
        
    };
    
}

#endif