#ifndef RAM_H
#define RAM_H

#include "tree.h"

namespace batchRam
{    
    class RandomAccessMemory
    {
    public:
        /**
         * The proxy for accessing a branch of the underlying tree
         * @param leafID
         * @return 
         */
        Branch getBranch(int leafID);
        
        /**
         * Chooses a random node and asks the underlying tree to return it.
         * 
         * @param level
         * @return 
         */
        Node getRandNodeAtLevel(int level);
        
        /**
         * Creates a branch from the 2PC and calls setBranch on memory
         * 
         * @param read2PC
         * @param wireIndexes
         * @param readBktIdx
         * @param nextLeafAccess
         */
        void update(Batch2PC rw2PC, std::vector<int> wireIndexes, int readBktIdx, int nextLeafAccess);
        
    private:
        Tree memory;
        // TODO implement stash once you implemented evict
//        Stash stash;

    };
}

#endif