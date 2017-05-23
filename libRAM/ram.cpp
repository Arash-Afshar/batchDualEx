
#include "ram.h"

namespace batchRam
{
    Branch
    RandomAccessMemory::getBranch(int leafID)
    {
        return memory.getBranch(leafID);
    }
    
    Node
    RandomAccessMemory::getRandNodeAtLevel(int level)
    {
        int nodePos = 0;
        // TODO: random position
        return memory.getNode(level, nodePos);
    }
    
    void
    RandomAccessMemory::update(Batch2PC &rw2PC, std::vector<int> wireIndexes, int readBktIdx, int nextLeafAccess)
    {
        
    }
}