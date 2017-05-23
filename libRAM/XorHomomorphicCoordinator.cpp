#include "XorHomomorphicCoordinator.h"
#include "constants.h"

namespace xhCoordinator
{
    
    
    XHCCoordinator::XHCCoordinator(osuCrypto::Role role)
    :
    mRole(role)
    {
    }
    
    XHCCoordinator::~XHCCoordinator()
    {
        end_point->stop();
        ios->stop();
    }
    
    void
    XHCCoordinator::xhcOfflinePhase(int numberOfComputations, int garbledCircuitOverhead, int totalIOCount){
        int port = 28001;
        int num_execs = 4; // TODO: should be a separate parameter read from user
        
//        perInputSize[0] = 256 * 3;
//        perInputSize[1] = 256 * 3;
//        perInputSize[2] = 256 * 3;
//        perInputSize[3] = 256 * 3;
//        perInputSize[4] = 256 * 3;
//        perInputSize[5] = 256 * 3;
//
//        perCircuitSize[0] = perInputSize[0] + 128 * 3;
//        perCircuitSize[1] = perInputSize[1] + 128 * 3;
//        perCircuitSize[2] = perInputSize[2] + 128 * 3;
//        perCircuitSize[3] = perInputSize[3] + 128 * 3;
//        perCircuitSize[4] = perInputSize[4] + 128 * 3;
//        perCircuitSize[5] = perInputSize[5] + 128 * 3;
//
//        startInRandCommit[0] = 0;
//        startInRandCommit[1] = perCircuitSize[0] * 100;
//        startInRandCommit[2] = perCircuitSize[1] * 100;
//        startInRandCommit[3] = perCircuitSize[2] * 100;
//        startInRandCommit[4] = perCircuitSize[3] * 100;
//        startInRandCommit[5] = perCircuitSize[4] * 100;
//        
//        outputStartOffset[0] = 33744;
//        outputStartOffset[1] = 33744;
//        outputStartOffset[2] = 33744;
//        outputStartOffset[3] = 33744;
//        outputStartOffset[4] = 33744;
//        outputStartOffset[5] = 33744;
//        
//        
//        int computationSize = 6;
//        actualOutputCommitments.resize(computationSize);
//        actualInputCommitments.resize(computationSize);
//        int totalCircuitCount = 8; // TODO
//        actualOutputCommitments[0].resize(totalCircuitCount); // total circuit count
//        actualOutputCommitments[1].resize(totalCircuitCount); // total circuit count
//        actualOutputCommitments[2].resize(totalCircuitCount); // total circuit count
//        actualOutputCommitments[3].resize(totalCircuitCount); // total circuit count
//        actualOutputCommitments[4].resize(totalCircuitCount); // total circuit count
//        actualOutputCommitments[5].resize(totalCircuitCount); // total circuit count
//        for(int j = 0; j < totalCircuitCount; j++)
//        {
//            actualOutputCommitments[0][j].resize(400); // outputsize
//            actualOutputCommitments[1][j].resize(400); // outputsize
//            actualOutputCommitments[2][j].resize(400); // outputsize
//            actualOutputCommitments[3][j].resize(400); // outputsize
//            actualOutputCommitments[4][j].resize(400); // outputsize
//            actualOutputCommitments[5][j].resize(400); // outputsize
//        }
//        
//        actualInputCommitments[0].resize(totalCircuitCount); // total circuit count
//        actualInputCommitments[1].resize(totalCircuitCount); // total circuit count
//        actualInputCommitments[2].resize(totalCircuitCount); // total circuit count
//        actualInputCommitments[3].resize(totalCircuitCount); // total circuit count
//        actualInputCommitments[4].resize(totalCircuitCount); // total circuit count
//        actualInputCommitments[5].resize(totalCircuitCount); // total circuit count
//        for(int j = 0; j < totalCircuitCount; j++)
//        {
//            actualInputCommitments[0][j].resize(400); // inputsize
//            actualInputCommitments[1][j].resize(400); // inputsize
//            actualInputCommitments[2][j].resize(400); // inputsize
//            actualInputCommitments[3][j].resize(400); // inputsize
//            actualInputCommitments[4][j].resize(400); // inputsize
//            actualInputCommitments[5][j].resize(400); // inputsize
//        }
//        
//
//        int num_commits = startInRandCommit[5] + perCircuitSize[5] * 100; // TODO changed it to match 5 + whatever 3 has
//        int num_commits = 230400;



        actualOutputCommitments.resize(numberOfComputations);
        actualInputCommitments.resize(numberOfComputations);
        for(int j = 0; j < numberOfComputations; j++){
            actualOutputCommitments[j].resize(garbledCircuitOverhead);
            actualInputCommitments[j].resize(garbledCircuitOverhead);
        }

        int num_commits = totalIOCount * 3 * garbledCircuitOverhead;
        std::cout << "Total commitments: " << num_commits << std::endl;
                
        std::string ip_address = "localhost"; // TODO: should be input parameter and the same as the one passed as program's input argument

        std::vector<std::future<void>> futures(2);
        thread_pool = new ctpl::thread_pool(std::thread::hardware_concurrency());

        ios = new osuCrypto::BtIOService(0);
        end_point = new osuCrypto::BtEndpoint(*ios, ip_address, port, mRole, "ep");
        osuCrypto::PRNG rnd;

        futures[0] = thread_pool->push([&](int id) {
            osuCrypto::Channel& send_channel = end_point->addChannel(chlIdStr("rand_commit_channel", mRole, true, true), chlIdStr("rand_commit_channel", mRole, true, false));
            rnd.SetSeed(load_block(constant_seeds[0].data()));

            SplitCommitSender base_sender;
            base_sender.SetMsgBitSize(128);
            base_sender.ComputeAndSetSeedOTs(rnd, send_channel);
            
            senders = new std::vector<SplitCommitSender>(num_execs);
            base_sender.GetCloneSenders(num_execs, *senders);
            sendRandomCommits(*senders, mRole, num_execs, num_commits);
            
            send_channel.close();            
        });

        futures[1] = thread_pool->push([&](int id) {
            osuCrypto::Channel& rec_channel = end_point->addChannel(chlIdStr("rand_commit_channel", mRole, false, true), chlIdStr("rand_commit_channel", mRole, false, false));
            rnd.SetSeed(load_block(constant_seeds[1].data()));

            SplitCommitReceiver base_receiver;
            base_receiver.SetMsgBitSize(128);
            base_receiver.ComputeAndSetSeedOTs(rnd, rec_channel);
            
            receivers = new std::vector<SplitCommitReceiver>(num_execs);
            std::vector<osuCrypto::PRNG> exec_rnds(num_execs);
            base_receiver.GetCloneReceivers(num_execs, rnd, *receivers, exec_rnds);
            receiveRandomCommits(*receivers, exec_rnds, mRole, num_execs, num_commits);

            rec_channel.close();            
        });

        futures[0].wait();
        futures[1].wait();
    }
    
    void
    XHCCoordinator::sendRandomCommits(std::vector<SplitCommitSender> &senders, osuCrypto::Role role, int num_execs, int num_commits)
    {
        std::vector<osuCrypto::Channel*> send_channels;
        for (int e = 0; e < num_execs; ++e) {            
            send_channels.emplace_back(&end_point->addChannel(chlIdStr("commit_channel_" + std::to_string(e), role, true, true), chlIdStr("commit_channel_" + std::to_string(e), role, true, false)));
        }

        std::vector<std::future<void> > futures(num_execs);
        uint32_t exec_num_commits = CEIL_DIVIDE(num_commits, num_execs);

        send_commit_shares = std::vector<std::array<BYTEArrayVector, 2> >(num_execs,{
            BYTEArrayVector(exec_num_commits, CODEWORD_BYTES),
            BYTEArrayVector(exec_num_commits, CODEWORD_BYTES)
        });

        for (int e = 0; e < num_execs; e++)
            futures[e] = thread_pool->push([&, e](int id) {
                senders[e].Commit(send_commit_shares[e], *send_channels[e]);
            });

        
        for (int e = 0; e < num_execs; e++){
            futures[e].wait();
            send_channels[e]->close();
        }
        
    }
    
    void
    XHCCoordinator::receiveRandomCommits(std::vector<SplitCommitReceiver> &receivers, std::vector<osuCrypto::PRNG> &exec_rnds, osuCrypto::Role role, int num_execs, int num_commits)
    {
        uint32_t exec_num_commits = CEIL_DIVIDE(num_commits, num_execs);
//        ctpl::thread_pool thread_pool(std::thread::hardware_concurrency());
        std::vector<std::future<void>> futures(num_execs);
        std::vector<osuCrypto::Channel*> rec_channels;
        rec_commit_shares = std::vector<BYTEArrayVector>(num_execs, BYTEArrayVector(exec_num_commits, CODEWORD_BYTES));
        
        for (int e = 0; e < num_execs; ++e) {
            rec_channels.emplace_back(&end_point->addChannel(chlIdStr("commit_channel_" + std::to_string(e), role, false, true), chlIdStr("commit_channel_" + std::to_string(e), role, false, false)));
        }

        for (int e = 0; e < num_execs; ++e) {
            futures[e] = thread_pool->push([&, e](int id) {
                receivers[e].Commit(rec_commit_shares[e], exec_rnds[e], *rec_channels[e]);
            });
        }

        for (int e = 0; e < num_execs; ++e) {
            futures[e].wait();
            rec_channels[e]->close();
        }
    }
    
    
    void
    XHCCoordinator::getPosInCommitShares(Identity id, int startOffset, int wireIndx, bool isInput, int &exec, int &offset)
    {
        int computationOffset = startInRandCommit[id.mComputationId];
        int circuitOffest = id.circuitOffset * perCircuitSize[id.mComputationId];
        int ioOffset;
        int wireOffset;
        
        if (isInput){
            ioOffset = 0;
            wireOffset = 3 * wireIndx;
        }else{
            ioOffset = perInputSize[id.mComputationId];
            int outputStart = wireIndx; // TODO: outputStartOffset[id.mComputationId];
            wireOffset =  3 * (wireIndx - outputStart);
        }
        int pos = computationOffset + circuitOffest + ioOffset + wireOffset + startOffset;

        int num_of_exec = send_commit_shares.size();
        int commits_per_exec = send_commit_shares[0][0].size();

        for (int e = 0; e < num_of_exec; e++){
            if (pos < (e + 1) * commits_per_exec){
                exec = e;
                offset = pos - e * commits_per_exec;
                break;
            }
        }
    }
    
    void
    XHCCoordinator::getRandomCommitment(Identity id, int wireIndx, bool isInput, std::array<uint8_t *, 6> &output)
    {
        assert(id.circuitOffset != -1);
        assert(id.mComputationId != -1);
        
        for (int i = 0; i < 3; i++){
            int exec = -1;
            int offset = -1;
//            getPosInCommitShares(id, i, wireIndx, isInput, exec, offset);
//            output[2 * i + 0] = send_commit_shares[exec][0][offset];
//            output[2 * i + 1] = send_commit_shares[exec][1][offset];
            // TODO, for the testing purposes, we use a fixed random commitment here
            output[2 * i + 0] = send_commit_shares[0][0][0];
            output[2 * i + 1] = send_commit_shares[0][1][0];
        }
        
    }
    
    void
    XHCCoordinator::commitToInput(std::vector<bool> permBit, std::vector<osuCrypto::block> allInputLabels, Identity id, osuCrypto::Channel &send_channel)
    {                
//        std::cout << osuCrypto::IoStream::lock << "sending \t" << id.toString() << " " << allInputLabels.size() / 2 << osuCrypto::IoStream::unlock << std::endl;
        std::vector<uint8_t> com;
        commitToIO(permBit, allInputLabels, id, send_channel, true, com);
    }
    
    void
    XHCCoordinator::receiveInputCommitments(Identity id, int inputSize, osuCrypto::Channel &rec_channel)
    {
        int length = inputSize * 3 * CODEWORD_BYTES;
        std::vector<uint8_t> com(length);
        rec_channel.recv(com.data(), sizeof(uint8_t) * length);

        std::unique_lock<std::mutex> lck (mtx,std::defer_lock);
        lck.lock();
        actualInputCommitments[id.mComputationId][id.circuitOffset] = std::move(com);
        lck.unlock();

    }
        
    void
    XHCCoordinator::commitToOutput(std::vector<bool> permBit, std::vector<osuCrypto::block> allOutputLabels, Identity id, osuCrypto::Channel &send_channel)
    {
        std::vector<uint8_t> com;
        commitToIO(permBit, allOutputLabels, id, send_channel, false, com);

    }

    void
    XHCCoordinator::receiveOutputCommitments(Identity id, int outputSize, osuCrypto::Channel &rec_channel)
    {
        int length = outputSize * 3 * CODEWORD_BYTES;
        std::vector<uint8_t> com(length);
        rec_channel.recv(com.data(), sizeof(uint8_t) * length);

        std::unique_lock<std::mutex> lck (mtx,std::defer_lock);
        lck.lock();
        actualOutputCommitments[id.mComputationId][id.circuitOffset] = std::move(com);
        lck.unlock();

    }
    
    void
    XHCCoordinator::commitToIO(std::vector<bool> permBit, std::vector<osuCrypto::block> allLabels, Identity id, osuCrypto::Channel &send_channel, bool isInput, std::vector<uint8_t> &actualIOCommitments)
    {
        actualIOCommitments.resize((allLabels.size()/2) * 3 * CODEWORD_BYTES);
        int pos = -1;
        for(uint64_t wire = 0; wire < allLabels.size() / 2; wire++)
        {
            std::array<uint8_t *, 6> randCommit;
            getRandomCommitment(id, wire, isInput, randCommit);
//            std::cout << wire << ", of " << actualIOCommitments.size() << ": " << id.circuitOffset << ": " << id.mComputationId << std::endl;
//            print("rand[0]:--->", randCommit[0], CODEWORD_BYTES);
            
            int b = 0;
            if(permBit[wire] == true){
                b = 1;
            }
            // commit to 0-key
            pos = 3 * wire * CODEWORD_BYTES;
            XOR_CodeWords( actualIOCommitments.data() + pos, randCommit[0], randCommit[1]);
            XOR_128(actualIOCommitments.data() + pos, (uint8_t*) &allLabels[2 * wire + b]);

            // commit to 1-key
            pos += CODEWORD_BYTES;
            XOR_CodeWords(actualIOCommitments.data() + pos, randCommit[2], randCommit[3]);
            XOR_128(actualIOCommitments.data() + pos, (uint8_t*) &allLabels[2 * wire + 1 - b]);            
            
            // commit to perm bit
            uint8_t bit = b;
            pos += CODEWORD_BYTES;
            XOR_CodeWords(actualIOCommitments.data() + pos, randCommit[4], randCommit[5]);
            XOR_UINT8_T(actualIOCommitments.data() + pos, &bit, 1);
        }

        send_channel.send(actualIOCommitments.data(), sizeof(uint8_t) * actualIOCommitments.size());
    }
    
    
    void
    XHCCoordinator::translateBucketHeads(Identity srcId, std::vector<uint64_t> outputWireIndexes, std::vector<osuCrypto::block> garbledOutputValue, Identity dstId, std::vector<uint64_t> inputWireIndexes, std::vector<std::vector<osuCrypto::block>> &garbledInputValue, uint64_t evalId)
    {
        std::string eId = std::to_string(evalId);
        assert(outputWireIndexes.size() == inputWireIndexes.size());
        
        uint64_t bucketSize = garbledInputValue.size();
        int commitPerBucket = 3 * outputWireIndexes.size();
        std::vector<uint8_t[CODEWORD_BYTES]> receivedXorCommitShares(3 * inputWireIndexes.size() * bucketSize);
        std::vector<std::future<void>> futures(2);

        futures[0] = thread_pool->push([&](int id) {
            osuCrypto::Channel& send_channel = end_point->addChannel(chlIdStr("decommit_channel"+eId, mRole, true, true), chlIdStr("decommit_channel"+eId, mRole, true, false));            

            std::vector<uint8_t[CODEWORD_BYTES]> outCommitShares(3 * outputWireIndexes.size() * bucketSize);
            std::vector<uint8_t[CODEWORD_BYTES]> inpCommitShares(3 * inputWireIndexes.size() * bucketSize);
            std::vector<uint8_t[CODEWORD_BYTES]> xorCommitShares(3 * inputWireIndexes.size() * bucketSize);
            
            for(uint64_t bs = 0; bs < bucketSize; bs++){
                Identity newSrcId(srcId.mComputationId, srcId.circuitOffset + bs, srcId.mRole);
                Identity newDstId(dstId.mComputationId, dstId.circuitOffset + bs, dstId.mRole);
                for(uint64_t j = 0; j < outputWireIndexes.size(); j++){
                    for (int i = 0; i < 3; i++){
                        int exec = -1;
                        int offset = -1;


                        // TODO: for now manually send r0 \oplus r1 until Decommit is changed to open a subset of commitments
    //                    (*senders)[exec].Decommit(send_commit_shares[exec], send_channel);
                        getPosInCommitShares(newSrcId, i, outputWireIndexes[j], false, exec, offset);
            //            ... = send_commit_shares[exec][0][offset];
            //            ... = send_commit_shares[exec][1][offset];
                        // TODO, for the testing purposes, we use a fixed random commitment here
                        XOR_CodeWords(outCommitShares[bs * commitPerBucket + j * 3 + i], send_commit_shares[0][0][0], send_commit_shares[0][1][0]);

                        getPosInCommitShares(newDstId, i, inputWireIndexes[j], true, exec, offset);
            //            ... = send_commit_shares[exec][0][offset];
            //            ... = send_commit_shares[exec][1][offset];
                        // TODO, for the testing purposes, we use a fixed random commitment here
    //                    input[0] = send_commit_shares[0][0][0];
    //                    input[1] = send_commit_shares[0][1][0];
                        XOR_CodeWords(inpCommitShares[bs * commitPerBucket + j * 3 + i], send_commit_shares[0][0][0], send_commit_shares[0][1][0]);


                        // TODO: check the permutation bit to find  which positions should be XORed together
                        XOR_CodeWords(xorCommitShares[bs * commitPerBucket + j * 3 + i], inpCommitShares[bs * commitPerBucket + j * 3 + i], outCommitShares[bs * commitPerBucket + j * 3 + i]);

                    }
                }
            }
            
            send_channel.asyncSend(xorCommitShares.data(), sizeof(uint8_t[CODEWORD_BYTES]) * xorCommitShares.size());
            send_channel.close();            
        });

        futures[1] = thread_pool->push([&](int id) {
            osuCrypto::Channel& rec_channel = end_point->addChannel(chlIdStr("decommit_channel"+eId, mRole, false, true), chlIdStr("decommit_channel"+eId, mRole, false, false));
            
            rec_channel.recv(receivedXorCommitShares.data(), sizeof(uint8_t[CODEWORD_BYTES]) * receivedXorCommitShares.size());
            
            rec_channel.close();
        });

        futures[0].get();
        futures[1].get();
        
        for(uint64_t circuitOffsetInBucket = 0; circuitOffsetInBucket < bucketSize; circuitOffsetInBucket++){
            int offsetInXor = circuitOffsetInBucket * commitPerBucket;
            
            garbledInputValue[circuitOffsetInBucket].resize(inputWireIndexes.size());
            
            for(uint64_t i = 0; i < garbledInputValue[circuitOffsetInBucket].size(); i++){
                // open the commitment on permutation bits
                int baseOutputOffset = outputWireIndexes[i] * 3 * CODEWORD_BYTES;
                int baseInputOffset = (inputWireIndexes[i] - inputWireIndexes[0]) * 3 * CODEWORD_BYTES; // since the actualInputCommitments only stores this parties commitments
                
                XOR_CodeWords(receivedXorCommitShares[offsetInXor + 3 * i + 2], actualOutputCommitments[srcId.mComputationId][srcId.circuitOffset].data() + baseOutputOffset + 2 * CODEWORD_BYTES);
                XOR_CodeWords(receivedXorCommitShares[offsetInXor + 3 * i + 2], actualInputCommitments[dstId.mComputationId][dstId.circuitOffset + circuitOffsetInBucket].data() + baseInputOffset + 2 * CODEWORD_BYTES);
                
                int permDelta = (receivedXorCommitShares[offsetInXor + 3 * i + 2][0] & 1);


                int outputOffset = baseOutputOffset + osuCrypto::PermuteBit(garbledOutputValue[outputWireIndexes[i]]) * CODEWORD_BYTES;
                int inputOffset = baseInputOffset + (osuCrypto::PermuteBit(garbledOutputValue[outputWireIndexes[i]]) ^ permDelta) * CODEWORD_BYTES;

                // TODO: the use of index i below is incorrect. It works now since we have essentially made receivedXorCommitShares to be always zero
                XOR_CodeWords(receivedXorCommitShares[offsetInXor + 3 * i + permDelta], actualOutputCommitments[srcId.mComputationId][srcId.circuitOffset].data() + outputOffset);
                XOR_CodeWords(receivedXorCommitShares[offsetInXor + 3 * i + permDelta], actualInputCommitments[srcId.mComputationId][srcId.circuitOffset + circuitOffsetInBucket].data() + inputOffset);
                xorBlockWithUint8(garbledInputValue[circuitOffsetInBucket][i], garbledOutputValue[outputWireIndexes[i]], receivedXorCommitShares[offsetInXor + 3 * i + permDelta]);

//                if(dstId == *(new Identity(1, 0, osuCrypto::First)) && circuitOffsetInBucket == 0 && i == 5){
//                    print("r0Inp:" + std::to_string(i) + ":\t", (uint8_t*)&garbledInputValue[circuitOffsetInBucket][i], 16);
//                }
//                if(dstId == *(new Identity(1, 0, osuCrypto::Second)) && circuitOffsetInBucket == 0){
//                    print("r1Inp:" + std::to_string(i) + ":\t", (uint8_t*)&garbledInputValue[circuitOffsetInBucket][i], 16);
//                }

    //            if(srcId == *(new Identity(0, 0, osuCrypto::Second))){
    //                print("inp[" + std::to_string(i) + "]:\t", (uint8_t*)&garbledInputValue[i], 16);
    //                print("out[" + std::to_string(i) + "]:\t", (uint8_t*)&garbledOutputValue[i], 16);
    //            }
            }
        }
        
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
    
    void
    xorBlockWithUint8(osuCrypto::block &dst, osuCrypto::block src, uint8_t *vec2)
    {
        int size = 16;
        uint8_t *dstUint = (uint8_t*)&dst;
        uint8_t *srcUint = (uint8_t*)&src;

        for (int i = 0; i < size; i++)
        {
            dstUint[i] = vec2[i] ^ srcUint[i];
        }
    }
    
//    void
//    xorUI8s(std::vector<uint8_t> &ret, int pos, uint8_t *vec1, int vec1_size, uint8_t *vec2, int vec2_size)
//    {
//        int vec_long_size = vec1_size;
//        int vec_small_size = vec2_size;
//        uint8_t *vec_long = vec1;
//        uint8_t *vec_small = vec2;
//
//        if (vec1_size < vec2_size)
//        {
//            vec_long_size = vec2_size;
//            vec_long = vec2;
//            vec_small_size = vec1_size;
//            vec_small = vec1;
//        }
//
////        if (ret.size() != vec_long_size)
////            ret.resize(vec_long_size);
//
//        for (int i = 0; i < vec_long_size; i++)
//        {
//    //        print(ret.data(), vec_long_size);
//            if (i < vec_small_size)
//                ret[pos + i] = vec_long[i] ^ vec_small[i];
//            else
//                ret[pos + i] = vec_long[i];
//        }
//    }
//    
//    void
//    xorUI8s(std::vector<uint8_t> &ret, int pos, int length, uint8_t *vec2, int vec2_size)
//    {
//        assert(length >= vec2_size);
//        for (int i = 0; i < length; i++)
//        {
//    //        print(ret.data(), vec_long_size);
////            if (i < vec2_size)
////                ret[pos + i] = ret[pos + i] ^ vec2[i];
////            else
//                ret[pos + i] = ret[pos + i];
//        }
//    }
    
    void
    print(std::string desc, uint8_t *vec, int vec_num_entries)
    {
        std::ostringstream convert;
        convert << desc << "\t";
        for (int a = vec_num_entries - 1; a >= 0 ; a--) {
            convert << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int) vec[a];
        }
        std::string key_string = convert.str();
        std::cout << key_string << std::endl;
    }

    std::string
    chlIdStr(std::string name, osuCrypto::Role role, bool isSender, bool isFrom)
    {
        std::string channelIdStr(name);
        if (isSender)
            if (isFrom)
                channelIdStr += osuCrypto::ToString(role) + osuCrypto::ToString(role);
            else
                channelIdStr += osuCrypto::ToString(role) + osuCrypto::ToString(1 - role);
        else
            if (isFrom)
                channelIdStr += osuCrypto::ToString(1 - role) + osuCrypto::ToString(role);
            else
                channelIdStr += osuCrypto::ToString(1 - role) + osuCrypto::ToString(1 - role);            
        return channelIdStr;
    }
    
}