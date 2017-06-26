
#include"coordinator.h"


namespace batchRam
{
    
    Coordinator::Coordinator(osuCrypto::Channel &myChl, osuCrypto::Role myRole, std::string circ_path_prefix, int memoryLength, int memoryCellLength, int instructionLength, int numExec, int bucketSize, int numOpened, int psiSecParam, int numConcurrentSetups, int numConcurrentEvals, int numThreadsPerEval)
    :
    chl(myChl),
    role(myRole)
    {
//        std::cout << "in Coord, going to configure" << std::endl;
        ramConfig::configure(memoryLength, memoryCellLength, instructionLength);
        xhcCoordinator = new xhCoordinator::XHCCoordinator(role);
//        std::cout << "going to init" << std::endl;
        initialize(circ_path_prefix, *xhcCoordinator, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval);
//        std::cout << "going to run main loop" << std::endl;
//        mainLoop();
    }
    
    void
    Coordinator::cleanup()
    {
        std::cout << "Cleanup called" << std::endl;
        chl.close();
    }
    void
    Coordinator::mainLoop()
    {
//        int t = 0;
//        while (t < executionTime)
//        {
//            if (t == 0)
//                initialLookup();
//            else
//                lookup(4 * t, universalInstruction2PC, t - 1, ramConfig::universalInstructionOutput_PC);
//            readMem(3 * t, 4 * t);
//            if (isHalt(t))
//                break; // TODO: this is not secure. It must continue running dummy operations until the end of loop
//            readArgs(t);
//            if (t == 0)
//                initialUniversalInstruction();
//            else
//                universalInstruction(4 * t);
//            lookup(4 * t + 3, universalInstruction2PC, t - 1, ramConfig::universalInstructionOutput_WDATA_ADDR);
//            writeMem(t, 4 * t + 3);
//            t++;
//        }
    }

    /* This is just for calculating the gate count*/
    void custom_read(std::string circ_path_prefix, int memSize, int dataLen, std::string file_name){
        std::string file = circ_path_prefix + "/" + file_name + "-" + std::to_string(memSize) + "-" + std::to_string(dataLen) + ".circ";
        std::fstream fStrm(file);
        if (fStrm.is_open() == false)
        {
            std::cout << "failed to open circuit file: " << file << std::endl;
            throw std::runtime_error("");
        }
        osuCrypto::Circuit cir;
        cir.readBris(fStrm);
        std::cout << file << "(ALL) "  << cir.Gates().size() << std::endl;
        std::cout << file << "(XOR) " << cir.NonXorGateCount() << std::endl;
    }
        
    /* This is just for calculating the gate count*/
    void custom_read(std::string circ_path_prefix, std::string file_name){
        custom_read(circ_path_prefix, ramConfig::N, ramConfig::dataLength, file_name);
    }
        

    void
    Coordinator::initialize(std::string circ_path_prefix, xhCoordinator::XHCCoordinator &xhcCoordinator, int numExec, int bucketSize, int numOpened, int psiSecParam, int numConcurrentSetups, int numConcurrentEvals, int numThreadsPerEval)
    {
        
//        lookup2PC.setXHCLib();
        // TODO: perform two lookups
        osuCrypto::Timer timer;

//        std::cout << "Offline" << std::endl;
        
        auto startXHCOffline = timer.setTimePoint("s_xhc_offline");
        int recursiveRatio = 4;
        int recursiveIterationsCount = std::log2(ramConfig::N) / std::log2(recursiveRatio) - 1;
        int rwCount = 4;
        int computationsInEachIteration = 2 + 2 * recursiveIterationsCount;
        int numberOfComputations = rwCount * computationsInEachIteration + 1;
//        int garbledCircuitOverhead = 2865; // assuming running the computation 1024 times and having 2 circuits per bucket
        int garbledCircuitOverhead = numExec * bucketSize;
        
        int memSize = ramConfig::N;
        int wordSize = ramConfig::dataLength;
        int readWriteIOCount = 2 * (ramConfig::elementsPerNode * (2 * ramConfig::logN + wordSize + 1) * (ramConfig::logN + 1)) + 2 * ramConfig::logN + wordSize;
        int evictIOCount =  2 * (ramConfig::elementsPerNode * (2 * ramConfig::logN + wordSize + 1) * (ramConfig::logN + 1)) + ramConfig::logN;
        int uiIOCount = ramConfig::instructionLength + 5 * (ramConfig::logN + ramConfig::dataLength) + 1;
        int totalIOCount = rwCount * (1 + recursiveIterationsCount) * (readWriteIOCount + evictIOCount) + uiIOCount;
  
        xhcCoordinator.xhcOfflinePhase(numberOfComputations, garbledCircuitOverhead, totalIOCount);
//        xhcCoordinator.xhcOfflinePhase(numberOfComputations, garbledCircuitOverhead, 10);
        auto endXHCOffline = timer.setTimePoint("s_xhc_offline");
              
//        for(int k = 0; k < rwCount; k++){ // read2PC and write2PC have the same computation overhead. Thus, I will use read2Pc inplace of both read2PC and write2PC
//            custom_read(circ_path_prefix, "read");
//            custom_read(circ_path_prefix, "evict");
//            
//            memSize = ramConfig::N;
//            for (int q = 0; q < recursiveIterationsCount; q++){
//                memSize = memSize / recursiveRatio;
//                wordSize = std::log2(memSize);
//                custom_read(circ_path_prefix, memSize, wordSize, "read");
//                custom_read(circ_path_prefix, memSize, wordSize, "evict");
//            }
//
//        }
//        custom_read(circ_path_prefix, "universalInstruction");
        
        
        auto startOffline = timer.setTimePoint("s_offline");
        for(int k = 0; k < rwCount; k++){ // read2PC and write2PC have the same computation overhead. Thus, I will use read2Pc inplace of both read2PC and write2PC
            int startID = computationsInEachIteration * k;
            read2PC.push_back(new   Read2PC(ramConfig::dataLength, circ_path_prefix, role, xhcCoordinator, startID + 0, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval));
            evict2PC.push_back(new Evict2PC(ramConfig::dataLength, circ_path_prefix, role, xhcCoordinator, startID + 1, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval));
            
            memSize = ramConfig::N;
            for (int q = 0; q < recursiveIterationsCount; q++){
                memSize = memSize / recursiveRatio;
                wordSize = std::log2(memSize);
                read2PC.push_back(new   Read2PC(wordSize, circ_path_prefix, role, xhcCoordinator, startID + 2 + 2 * q + 0, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval));
                evict2PC.push_back(new Evict2PC(wordSize, circ_path_prefix, role, xhcCoordinator, startID + 2 + 2 * q + 1, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval));
            }

        }
        universalInstruction2PC = new UniversalInstruction2PC(circ_path_prefix, role, xhcCoordinator, rwCount * computationsInEachIteration, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval);
        auto endOffline = timer.setTimePoint("e_offline");
//        std::cout << "Init Online" << std::endl;
        auto startInitOnline = timer.setTimePoint("s_initOnline");
        for(int k = 0; k < rwCount * (1 + recursiveIterationsCount); k++){
            read2PC[k]->initEvaluate();
            evict2PC[k]->initEvaluate();
        }
        universalInstruction2PC->initEvaluate();
        
        
//        lookup2PC->initEvaluate();
//        read2PC->initEvaluate();
//        write2PC->initEvaluate();
//        halt2PC->initEvaluate();
//        universalInstruction2PC->initEvaluate();
//        evict2PC->initEvaluate();
        auto endInitOnline = timer.setTimePoint("e_initOnline");
        
        std::vector<osuCrypto::block> FIX_ME_GarbledOutputs;
//        std::vector<osuCrypto::block> readGarbledOutputs;
//        std::vector<osuCrypto::block> uiGarbledOutputs;
//        Identity readBucketHeadId;
//        Identity uiBucketHeadId;
//        std::vector<uint64_t> outputWireIndexes;
//        std::vector<uint64_t> inputWireIndexes;
        
//        std::cout << "Online" << std::endl;
        auto startOnline = timer.setTimePoint("s_online");
        for (osuCrypto::u64 i = 0; i < static_cast<osuCrypto::u64>(numExec); i += numConcurrentEvals)
        {
//////            lookup2PC->evaluate(i, lookupGarbledOutputs, *(new std::vector<uint64_t>(0)), *(new std::vector<std::vector<osuCrypto::block>>(0)));
//////            
//////            lookupBucketHeadId = lookup2PC->getBucketHeadId(i);
//////            readBucketHeadId = read2PC->getBucketHeadId(i);
//////            outputWireIndexes = lookup2PC->getRelativeOutputWireIndexes();
//////            inputWireIndexes = read2PC->getRelativeInputWireIndexes();
//////            std::vector<std::vector<osuCrypto::block>> garbledInputValue(bucketSize);
//////            xhcCoordinator.translateBucketHeads(lookupBucketHeadId, outputWireIndexes, lookupGarbledOutputs, readBucketHeadId, inputWireIndexes, garbledInputValue, i);
////
////            read2PC->evaluate(i, readGarbledOutputs, *(new std::vector<uint64_t>(0)), *(new std::vector<std::vector<osuCrypto::block>>(0)));
//////            read2PC->evaluate(i, readGarbledOutputs, inputWireIndexes, garbledInputValue);
//////            write2PC->evaluate(i, readGarbledOutputs, *(new std::vector<uint64_t>(0)), *(new std::vector<std::vector<osuCrypto::block>>(0)));
//////            halt2PC->evaluate(i, readGarbledOutputs, *(new std::vector<uint64_t>(0)), *(new std::vector<std::vector<osuCrypto::block>>(0)));
//////            universalInstruction2PC->evaluate(i, readGarbledOutputs, *(new std::vector<uint64_t>(0)), *(new std::vector<std::vector<osuCrypto::block>>(0)));
//////            evict2PC->evaluate(i, readGarbledOutputs, *(new std::vector<uint64_t>(0)), *(new std::vector<std::vector<osuCrypto::block>>(0)));
//            
//            read2PC->evaluate(i, readGarbledOutputs, *(new std::vector<uint64_t>(0)), *(new std::vector<std::vector<osuCrypto::block>>(0)));
//            readBucketHeadId = read2PC->getBucketHeadId(i);
//            uiBucketHeadId = universalInstruction2PC->getBucketHeadId(i);            
//            outputWireIndexes = read2PC->getOutputSegmentWireIndexes("RDATA");
//            inputWireIndexes = universalInstruction2PC->getInputSegmentWireIndexes("ARG_DATA1");
//            std::vector<std::vector<osuCrypto::block>> garbledInputValue(bucketSize);
//            xhcCoordinator.translateBucketHeads(readBucketHeadId, outputWireIndexes, readGarbledOutputs, uiBucketHeadId, inputWireIndexes, garbledInputValue, i);
//            std::cout << "------- Iteration: " << i << std::endl;
//            universalInstruction2PC->evaluate(i, uiGarbledOutputs, inputWireIndexes, garbledInputValue);
//            std::cout << "------- Iteration: success" << std::endl;
                        
           for(int k = 0; k < rwCount * (1 + recursiveIterationsCount); k++){
                read2PC[k]->evaluate(i, FIX_ME_GarbledOutputs, *(new std::vector<uint64_t>(0)), *(new std::vector<std::vector<osuCrypto::block>>(0)));
                
//                Identity fstBktHdId1 = read2PC[k]->getBucketHeadId(i);
//                Identity sndBktHdId1 = write2PC[k]->getBucketHeadId(i);            
//                std::vector<uint64_t> outputWireIndexes1 = read2PC[k]->getOutputSegmentWireIndexes("BRANCH");
//                std::vector<uint64_t> inputWireIndexes1 = write2PC[k]->getInputSegmentWireIndexes("BRANCH");
//                std::vector<std::vector<osuCrypto::block>> garbledInputValue1(bucketSize);
//                xhcCoordinator.translateBucketHeads(fstBktHdId1, outputWireIndexes1, FIX_ME_GarbledOutputs, sndBktHdId1, inputWireIndexes1, garbledInputValue1, i);

                evict2PC[k]->evaluate(i, FIX_ME_GarbledOutputs, *(new std::vector<uint64_t>(0)), *(new std::vector<std::vector<osuCrypto::block>>(0)));
           }
           universalInstruction2PC->evaluate(i, FIX_ME_GarbledOutputs, *(new std::vector<uint64_t>(0)), *(new std::vector<std::vector<osuCrypto::block>>(0)));
        }
        auto endOnline = timer.setTimePoint("e_online");
        
        for(int k = 0; k < rwCount * (1 + recursiveIterationsCount); k++){
            read2PC[k]->cleanup();
            evict2PC[k]->cleanup();
        }
        universalInstruction2PC->cleanup();
        
//        lookup2PC->cleanup();
//        read2PC->cleanup();
//        write2PC->cleanup();
//        halt2PC->cleanup();
//        universalInstruction2PC->cleanup();
//        evict2PC->cleanup();


        auto xhcoffline = std::chrono::duration_cast<std::chrono::microseconds>(endXHCOffline - startXHCOffline).count();
        auto offline = std::chrono::duration_cast<std::chrono::microseconds>(endOffline - startOffline).count();
        auto initOnline = std::chrono::duration_cast<std::chrono::microseconds>(endInitOnline - startInitOnline).count();
        auto online = std::chrono::duration_cast<std::chrono::microseconds>(endOnline - startOnline).count();
        std::cout << osuCrypto::IoStream::lock << "total xhc_offline  = " << xhcoffline / 1000.0 << " ms" << osuCrypto::IoStream::unlock << std::endl;
        std::cout << osuCrypto::IoStream::lock << "total offline      = " << offline / 1000.0 << " ms" << osuCrypto::IoStream::unlock << std::endl;
        std::cout << osuCrypto::IoStream::lock << "total init_online  = " << initOnline / 1000.0 << " ms" << osuCrypto::IoStream::unlock << std::endl;
        std::cout << osuCrypto::IoStream::lock << "total online       = " << online / 1000.0 << " ms" << osuCrypto::IoStream::unlock << std::endl;
        std::cout << osuCrypto::IoStream::lock << "each  online       = " << (online / 1000.0) / numExec << " ms" << osuCrypto::IoStream::unlock << std::endl;

    }

    void
    Coordinator::initialLookup()
    {
//        int bucketIdx = 0;
//        int directInputLengths[] = {2 * ramConfig::logN + ramConfig::N * ramConfig::logN, ramConfig::logN};
//        lookup2PC.setDirectInputWires(bucketIdx, directInputLengths);
//        lookup2PC.prepareInputs(bucketIdx);
//        lookup2PC.evaluate(bucketIdx);
//        nextLeafAccess = lookup2PC.getWiresAsPlainNumber(bucketIdx, ramConfig::logN);
//        prevLookupIdx = bucketIdx;
    }

    void
    Coordinator::lookup(int bucketIdx, Batch2PC &src, int srcBktIdx, std::vector<int> srcInputWireIndexes)
    {
//        int directInputLengths[] = {ramConfig::logN, ramConfig::logN};
//        lookup2PC.setDirectInputWires(bucketIdx, directInputLengths);
//        connectBktToBkt(src, srcBktIdx, srcInputWireIndexes, lookup2PC, bucketIdx, ramConfig::lookupInput_REAL_DATA_ID);
//        connectBktToBkt(lookup2PC, prevLookupIdx, ramConfig::lookupInput_TABLE, lookup2PC, bucketIdx, ramConfig::lookupInput_TABLE);
//        lookup2PC.prepareInputs(bucketIdx);
//        lookup2PC.evaluate(bucketIdx);
//        nextLeafAccess = lookup2PC.getWiresAsPlainNumber(bucketIdx, ramConfig::logN);
//        prevLookupIdx = bucketIdx;
    }

    void
    Coordinator::readMem(int readBktIdx, int lookupBktIdx)
    {
//        connectBktToBkt(lookup2PC, lookupBktIdx, ramConfig::lookupOutput_REAL_DATA_ID, read2PC, readBktIdx, ramConfig::readInput_REAL_DATA_ID);
//        connectBktToBkt(lookup2PC, lookupBktIdx, ramConfig::lookupOutput_NEW_LEAF_ID, read2PC, readBktIdx, ramConfig::readInput_NEW_LEAF_ID);
//        connectRamToBkt(read2PC, readBktIdx, ramConfig::readInput_BRANCH);
//        read2PC.evaluate(readBktIdx);
//        ram.update(read2PC, ramConfig::readOutput_BRANCH, readBktIdx, nextLeafAccess);
//        evict();
    }

    void
    Coordinator::writeMem(int writeBktIdx, int lookupBktIdx)
    {
//        int universalInstructionBkIdx = writeBktIdx;
//        connectBktToBkt(lookup2PC, lookupBktIdx, ramConfig::lookupOutput_REAL_DATA_ID, write2PC, writeBktIdx, ramConfig::writeInput_REAL_DATA_ID);
//        connectBktToBkt(lookup2PC, lookupBktIdx, ramConfig::lookupOutput_NEW_LEAF_ID, write2PC, writeBktIdx, ramConfig::writeInput_NEW_LEAF_ID);
//        connectBktToBkt(universalInstruction2PC, universalInstructionBkIdx, ramConfig::universalInstructionOutput_WDATA, write2PC, writeBktIdx, ramConfig::writeInput_WDATA);
//        connectRamToBkt(write2PC, writeBktIdx, ramConfig::writeInput_BRANCH);
//        write2PC.evaluate(writeBktIdx);
//        ram.update(write2PC, ramConfig::writeOutput_BRANCH, writeBktIdx, nextLeafAccess);
//        evict();
    }

    void
    Coordinator::readArgs(int t)
    {
//        int bktIdx = 0;
//
//        bktIdx = 4 * t + 1 + 0;
//        lookup(bktIdx, read2PC, 3 * t, ramConfig::readOutput_DATA_ARG1);
//        readMem(3 * t + 1, bktIdx);
//
//        bktIdx = 4 * t + 1 + 1;
//        lookup(bktIdx, read2PC, 3 * t, ramConfig::readOutput_DATA_ARG2);
//        readMem(3 * t + 2, bktIdx);
    }

    bool
    Coordinator::isHalt(int bucketIdx)
    {
    //    connectBktToBkt(lookup2PC, lookupBktIdx, realLeafWireIndexes, read2PC, readBktIdx, someIndexes);
    //    connectBktToBkt(lookup2PC, lookupBktIdx, newLeafWireIndexes, read2PC, readBktIdx, someOtherIndexes);
        // TODO:
        //    connect PC (e.g. read at 3 * t)
        //    connect last read data (e.g. universal instruction at t)
        return false;
    }

    void
    Coordinator::commonUniversalInstructionSetup(int uiBucketIdx)
    {
//        // connect arguments
//        int t = uiBucketIdx;
//        connectBktToBkt(read2PC, 3 * t + 1, ramConfig::readOutput_DATA, universalInstruction2PC, uiBucketIdx, ramConfig::universalInstructionInput_ARG1);
//        connectBktToBkt(read2PC, 3 * t + 2, ramConfig::readOutput_DATA, universalInstruction2PC, uiBucketIdx, ramConfig::universalInstructionInput_ARG2);
//
//        // connect instruction
//        connectBktToBkt(read2PC, 3 * t + 0, ramConfig::readOutput_DATA_INSTRUCTION, universalInstruction2PC, uiBucketIdx, ramConfig::universalInstructionInput_INSTRUCTION);
    }

    void
    Coordinator::initialUniversalInstruction()
    {
//        Coordinator::commonUniversalInstructionSetup(0);
//        // The input should always be 0 since PC is initially in the first memory location
//        // Does not matter which party sets it, we are assuming that it initial phase is done honestly anyway!
//        int directInputLengths[] = {ramConfig::logN, 0};
//        universalInstruction2PC.setDirectInputWires(0, directInputLengths);
//
//        // The input should always be 0 since the r_data  is initially all zeros
//        // Does not matter which party sets it, we are assuming that it initial phase is done honestly anyway!
//        directInputLengths[0] = ramConfig::dataLength;
//        universalInstruction2PC.setDirectInputWires(0, directInputLengths);
//        universalInstruction2PC.evaluate(0);
    }


    void
    Coordinator::universalInstruction(int bucketIdx)
    {
//        Coordinator::commonUniversalInstructionSetup(bucketIdx);
//        // connect PC
//        connectBktToBkt(universalInstruction2PC, bucketIdx - 1, ramConfig::universalInstructionOutput_PC, universalInstruction2PC, bucketIdx, ramConfig::universalInstructionInput_PC);
//        // connect rdata
//        connectBktToBkt(universalInstruction2PC, bucketIdx - 1, ramConfig::universalInstructionOutput_RDATA, universalInstruction2PC, bucketIdx, ramConfig::universalInstructionInput_RDATA);
//        universalInstruction2PC.evaluate(bucketIdx);
    }


    //---------------------------------------------------------------------------------------------------------------------------------

    void
    Coordinator::connectBktToBkt(Batch2PC &src, int srcBktIdx, std::vector<int> srcWireIndexes, Batch2PC &dst, int dstBktIdx, std::vector<int> dstWireIndexes)
    {
//        assert(std::is_same(srcWireIndexes.size(), dstWireIndexes.size()));
//
//        // Both this coordinator and the other one know what needs to be opened (based on the inputs of this function)
//        // Therefore, they simply send the corresponding openings to the other coordinator
//
//        std::vector<std::array<xhCoordinator::XorHomomorphicCommit, 2> >  srcWireCommitments = src.getXHCommitments(srcBktIdx, srcWireIndexes);
//        std::vector<xhCoordinator::XorHomomorphicCommit> srcPointNPermuteCommitment = src.getXHPointNPermuteCommitments(srcBktIdx, srcWireIndexes);
//
//        std::vector<std::array<xhCoordinator::XorHomomorphicCommit, 2> >  dstWireCommitments = dst.getXHCommitments(dstBktIdx, dstWireIndexes);
//        std::vector<xhCoordinator::XorHomomorphicCommit> dstPointNPermuteCommitment = dst.getXHPointNPermuteCommitments(dstBktIdx, dstWireIndexes);
//
//        std::vector<std::array<osuCrypto::block, 2> > otherDeltas = xhCoordinator::computeDeltas(srcWireCommitments, dstWireCommitments);
//        std::vector<osuCrypto::block> otherPointNPermuteDeltas = xhCoordinator::computeDeltas(srcPointNPermuteCommitment, dstPointNPermuteCommitment);
//
//        std::vector<std::array<osuCrypto::block, 2> > deltas(otherDeltas.size());
//        std::vector<osuCrypto::block> pointNPermuteDeltas(otherPointNPermuteDeltas.size());
//
//        assert(otherPointNPermuteDeltas.size() == otherDeltas.size());
//        osuCrypto::u64 size = sizeof(std::array<osuCrypto::block, 2>) * otherDeltas.size() + sizeof(osuCrypto::block) * otherPointNPermuteDeltas.size();
//        std::unique_ptr<osuCrypto::ByteStream> buff(new osuCrypto::ByteStream(size));
//        buff->setp(size);
//        osuCrypto::block* deltaStream = (osuCrypto::block*)buff->data();
//
//        for (osuCrypto::u64 i = 0; i < otherDeltas.size(); ++i)
//        {
//            *deltaStream++ = otherDeltas[i][0];
//            *deltaStream++ = otherDeltas[i][1];
//            *deltaStream++ = otherPointNPermuteDeltas[i];
//        }
//        // TODO fix network
////        chl_snd.asyncSend(std::move(buff));
//        buff.release();
////        chl_rcv.recv(buff->data(), size);
//        deltaStream = (osuCrypto::block*)buff->data();
//        
//        for (osuCrypto::u64 i = 0; i < otherDeltas.size(); ++i)
//        {
//            std::array<osuCrypto::block, 2> deltasOfDecommits;
//            deltasOfDecommits[0] = (osuCrypto::block)(*deltaStream);
//            deltaStream++;
//            deltasOfDecommits[1] = (osuCrypto::block)(*deltaStream);
//            deltaStream++;
//            deltas[i] = deltasOfDecommits;
//            
//            otherPointNPermuteDeltas[i] = (osuCrypto::block)(*deltaStream);
//            deltaStream++;
//        }
//
//        // both the commitments on wires and the point-n-permute commitments
//        std::vector<std::pair<osuCrypto::block, std::array<xhCoordinator::XorHomomorphicCommit, 2> > >  srcWireAndCommitments = src.getWireAndXHCommitments(srcBktIdx, srcWireIndexes);
//        std::vector<std::pair<osuCrypto::block, std::array<xhCoordinator::XorHomomorphicCommit, 2> > >  dstWireAndCommitments = dst.getWireAndXHCommitments(dstBktIdx, dstWireIndexes);
//
//
//        if (xhCoordinator::verify(deltas, pointNPermuteDeltas, srcWireAndCommitments, dstWireAndCommitments))
//        {
//            std::vector<osuCrypto::block> dstGarbledInputs = convert(deltas, srcWireAndCommitments);
//            dst.setXHCInputs(dstBktIdx, dstWireIndexes, dstGarbledInputs);
//        }
    }
    
    std::vector<osuCrypto::block>
    Coordinator::convert(std::vector<std::array<osuCrypto::block, 2> > deltas, std::vector<std::pair<osuCrypto::block, std::array<xhCoordinator::XorHomomorphicCommit, 2> > >  srcWireAndCommitments)
    {
        std::vector<osuCrypto::block> dstGarbledValues(srcWireAndCommitments.size());
//        osuCrypto::u16 one = 1;
//        for (osuCrypto::u64 i = 0; i < srcWireAndCommitments.size(); i++)
//        {
//            osuCrypto::block src = srcWireAndCommitments[i].first;
//            osuCrypto::u16 *bitVector = (osuCrypto::u16*) &src;
//            int pnpBit = bitVector[0] ^ one;
//            dstGarbledValues[i] = src ^ deltas[i][pnpBit];
//        }
        return dstGarbledValues;
    }
    
    void
    Coordinator::connectRamToBkt(Batch2PC &dst, int srcBktIdx, std::vector<int> dstWireIndexes)
    {
//        // as in connectBktToBkt, the parties know what needs to be connected based on wireIndexes and nextLeafAccess
//        batchRam::Branch branch = ram.getBranch(nextLeafAccess);
//        
//        // srcWireCommitments -> 1st dimension: all the nodes, 2nd dimension: elements inside a node, 3rd dimension wires of an element, 4th dimension (e.g. the pair) actual commitment & the point-and-permute commitment
//        std::vector<std::vector<std::vector<std::pair<xhCoordinator::XorHomomorphicCommit, xhCoordinator::XorHomomorphicCommit> > > >  srcWireCommitments = branch.getXHCommitments();
//        
//        // TODO: the rest is the same as connectBktToBkt. Extract it into a function
    }
    
    void
    Coordinator::evict()
    {
        
    }
    
    void print(std::string desc, uint8_t *vec, int vec_num_entries)
    {
        std::ostringstream convert;
        convert << desc << "\t";
        for (int a = 0; a < vec_num_entries; a++) {
            convert << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int) vec[a];
        }
        std::string key_string = convert.str();
        std::cout << key_string << std::endl;
    }

}