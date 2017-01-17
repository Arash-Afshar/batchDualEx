#include "tree.h"

namespace batchRam
{
    
    Node::Node(std::vector<GarbledData> nodeData)
    {
        capacity = ramConfig::elementsPerNode;
        setGarbledNode(nodeData);
    }

    void
    Node::setGarbledNode(std::vector<GarbledData> nodeData)
    {
        this->nodeData = nodeData;
    }

    std::vector<std::vector<std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit> > >
    Node::getCommitments()
    {
        std::vector<std::vector<std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit> > > commitments;
        for (int i = 0; i < capacity; i++)
        {
            int j = 0;
            for (; j < ramConfig::dataLength; j++)
                commitments[i][j] = nodeData[i].dataCommitment[j];

            for (; j < ramConfig::dataLength + ramConfig::leafIdLength; j++)
                commitments[i][j] = nodeData[i].realIDCommitment[j - ramConfig::dataLength];

            for (; j < ramConfig::dataLength + ramConfig::leafIdLength * 2; j++)
                commitments[i][j] = nodeData[i].leafIDCommitment[j- ramConfig::dataLength - ramConfig::leafIdLength];

        }
        return commitments;
    } 

    std::vector<std::vector<osuCrypto::block> >
    Node::getGarbledValues()
    {
        std::vector<std::vector<osuCrypto::block > > garbledValues;
        for (int i = 0; i < capacity; i++)
        {
            int j = 0;
            for (; j < ramConfig::dataLength; j++)
                garbledValues[i][j] = nodeData[i].data[j];

            for (; j < ramConfig::dataLength + ramConfig::leafIdLength; j++)
                garbledValues[i][j] = nodeData[i].realID[j - ramConfig::dataLength];

            for (; j < ramConfig::dataLength + ramConfig::leafIdLength * 2; j++)
                garbledValues[i][j] = nodeData[i].leafID[j- ramConfig::dataLength - ramConfig::leafIdLength];

        }
        return garbledValues;
    }
    
    
    Branch::Branch(Node root, int leafId)
    {
        height = ramConfig::logN;

        directions = std::vector<bool>(height);
        
        //TODO: convert leaf to bitvector of directions
        Node node = root;

        for (osuCrypto::u64 i = 0; i < directions.size(); i ++)
        {
            nodes[i] = node;
            node = node.children[directions[i]];
        }
    }

    std::pair<Node, int>
    Branch::getNodeAt(int i)
    {
        return std::pair<Node, bool>(nodes[i], directions[i]);
    }

    std::vector<std::vector<std::vector<std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit> > > >
    Branch::getXHCommitments()
    {
        
        std::vector<std::vector<std::vector<std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit> > > > allXHCommitments(nodes.size());
        for (int i = 0; nodes.size(); i++)
        {
            allXHCommitments[i] = nodes[i].getCommitments();
        }
        return allXHCommitments;
    }
    
    Tree::Tree()
    {
        height = ramConfig::logN;

        levels.resize(height);
        levels[0] = std::vector<Node>(1);
        levels[0].resize(1);
        levels[0][0] = root;
        for (int level = 0; level < height - 1; level++)
        {
            levels[level + 1] = std::vector<Node>();
            for (osuCrypto::u64 i = 0; i < levels[level].size(); i++)
            {
                levels[level][i].children = new Node[2];
//                levels[level][i].children[0] = new Node();
//                levels[level][i].children[1] = new Node();
                levels[level + 1].push_back(levels[level][i].children[0]);
                levels[level + 1].push_back(levels[level][i].children[1]);
            }
        }
    }
    
    Branch
    Tree::getBranch(int leafId)
    {
        return Branch(this->root, leafId);
    }
    
    void
    Tree::setBranch(int leafId, Branch branch)
    {
        Node node = root;
        for (int i = 0; i < height; i++)
        {
            std::pair<Node, int> nodePos = branch.getNodeAt(i);
            // TODO Fix the following
            node.setGarbledNode(nodePos.first.getGarbledNode());
            node = node.children[nodePos.second];
        }
    }
    
    Node
    Tree::getNode(int level, int nodePos)
    {
        return levels[level][nodePos];
    }

}