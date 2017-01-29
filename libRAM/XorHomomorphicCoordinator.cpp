#include "XorHomomorphicCoordinator.h"

namespace xhCoordinator
{
    
    
    XHCCoordinator::XHCCoordinator(int num_commits, osuCrypto::Role role)
    {
        int port = 28001;
        int num_execs = 4; // TODO: should be a separate parameter read from user
        
        // TODO: input is expanded due to kprob ---> add correct input size
        perInputSize[0] = 256 * 3;
        perInputSize[1] = 256 * 3;
        perInputSize[2] = -100000 * 3;
        perInputSize[3] = -100000 * 3;

        perCircuitSize[0] = perInputSize[0] + 128 * 3;
        perCircuitSize[1] = perInputSize[1] + 128 * 3;
        perCircuitSize[2] = perInputSize[2] + -100000 * 3;
        perCircuitSize[3] = perInputSize[3] + -100000 * 3;

        startInRandCommit[0] = 0;
        startInRandCommit[1] = perCircuitSize[0] * 100;
        startInRandCommit[2] = perCircuitSize[1] * 100;
        startInRandCommit[3] = -1000000;
        
        outputStartOffset[0] = 33744;
        outputStartOffset[1] = 33744;
        outputStartOffset[2] = -1000000;
        outputStartOffset[3] = -1000000;
        
        
        num_commits = startInRandCommit[2]; // TODO changed it to match 3 + whatever 3 has
        std::string ip_address = "localhost"; // TODO: should be input parameter and the same as the one passed as program's input argument

        std::vector<std::future<void>> futures(2);
        thread_pool = new ctpl::thread_pool(std::thread::hardware_concurrency());

        osuCrypto::BtIOService ios(0);
        end_point = new osuCrypto::BtEndpoint(ios, ip_address, port, role, "ep");
        osuCrypto::PRNG rnd;

        futures[0] = thread_pool->push([&](int id) {
            osuCrypto::Channel& send_ot_channel = end_point->addChannel(chlIdStr("ot_channel", role, true, true), chlIdStr("ot_channel", role, true, false));
            rnd.SetSeed(load_block(constant_seeds[0].data()));

            SplitCommitSender base_sender;
            base_sender.SetMsgBitSize(128);
            base_sender.ComputeAndSetSeedOTs(rnd, send_ot_channel);
            
            std::vector<SplitCommitSender> senders(num_execs);
            base_sender.GetCloneSenders(num_execs, senders);
            sendeRandomCommits( senders, role, num_execs, num_commits);
            
            send_ot_channel.close();            
        });

        futures[1] = thread_pool->push([&](int id) {
            osuCrypto::Channel& rec_ot_channel = end_point->addChannel(chlIdStr("ot_channel", role, false, true), chlIdStr("ot_channel", role, false, false));
            rnd.SetSeed(load_block(constant_seeds[1].data()));

            SplitCommitReceiver base_receiver;
            base_receiver.SetMsgBitSize(128);
            base_receiver.ComputeAndSetSeedOTs(rnd, rec_ot_channel);
            
            std::vector<SplitCommitReceiver> receivers(num_execs);
            std::vector<osuCrypto::PRNG> exec_rnds(num_execs);
            base_receiver.GetCloneReceivers(num_execs, rnd, receivers, exec_rnds);
            receiveRandomCommits(receivers, exec_rnds, role, num_execs, num_commits);

            rec_ot_channel.close();            
        });

        futures[0].wait();
        futures[1].wait();

        end_point->stop();
        ios.stop();
    }
    
    void
    XHCCoordinator::sendeRandomCommits(std::vector<SplitCommitSender> &senders, osuCrypto::Role role, int num_execs, int num_commits)
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
    XHCCoordinator::getRandomCommitment(Identity id, int wireIndx, bool isInput, std::array<uint8_t *, 6> &output)
    {
        assert(id.circuitOffset != -1);
        assert(id.mComputationId != -1);
        
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
        int pos = computationOffset + circuitOffest + ioOffset + wireOffset;
        
        int num_of_exec = send_commit_shares.size();
        int commits_per_exec = send_commit_shares[0][0].size();

        for (int i = 0; i < 3; i++){
            int exec = -1;
            int offset = -1;
            for (int e = 0; e < num_of_exec; e++){
                if (pos + i < (e + 1) * commits_per_exec){
                    exec = e;
                    offset = pos + i - e * commits_per_exec;
                    break;
                }
            }
//            std::cout << "pos: " << pos << ", exec: " << exec << ", offset: " << offset << ", of "  << commits_per_exec << std::endl;
            output[2 * i + 0] = send_commit_shares[exec][0][offset];
            output[2 * i + 1] = send_commit_shares[exec][1][offset];
        }
        
    }
    
    void
    XHCCoordinator::commitToInput(std::vector<bool> permBit, std::vector<osuCrypto::block> allInputLabels, Identity id, osuCrypto::Channel &send_channel)
    {
        std::vector<uint8_t> inputCommitments;
        commitToIO(permBit, allInputLabels, id, send_channel, true, inputCommitments);
    }
    
    void
    XHCCoordinator::receiveInputCommitments(Identity id, int inputSize, osuCrypto::Channel &rec_channel)
    {
        std::vector<uint8_t> inputCommitments(inputSize * 2 * 3 * CODEWORD_BYTES);
        rec_channel.recv(inputCommitments.data(), sizeof(uint8_t) * inputCommitments.size());
    }
        
    void
    XHCCoordinator::commitToOutput(std::vector<bool> permBit, std::vector<osuCrypto::block> allOutputLabels, Identity id, osuCrypto::Channel &send_channel)
    {
        std::vector<uint8_t> outputCommitments;
        commitToIO(permBit, allOutputLabels, id, send_channel, false, outputCommitments);
    }

    void
    XHCCoordinator::receiveOutputCommitments(Identity id, int outputSize, osuCrypto::Channel &rec_channel)
    {
        std::vector<uint8_t> outputCommitments(outputSize * 2 * 3 * CODEWORD_BYTES);
        rec_channel.recv(outputCommitments.data(), sizeof(uint8_t) * outputCommitments.size());
    }
    
    void
    XHCCoordinator::commitToIO(std::vector<bool> permBit, std::vector<osuCrypto::block> allLabels, Identity id, osuCrypto::Channel &send_channel, bool isInput, std::vector<uint8_t> &actualIOCommitments)
    {
        actualIOCommitments.resize((allLabels.size()/2) * 2 * 3 * CODEWORD_BYTES);
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
            uint8_t bit = b;
            // commit to 0-key
            pos = 6 * wire * CODEWORD_BYTES;
            xorUI8s(actualIOCommitments, pos, randCommit[0], CODEWORD_BYTES, randCommit[1], CODEWORD_BYTES);
            pos += CODEWORD_BYTES;
            xorUI8s(actualIOCommitments, pos, CODEWORD_BYTES, (uint8_t*) &allLabels[2 * wire + b], CSEC_BYTES);

            // commit to 1-key
            pos += CODEWORD_BYTES;
            xorUI8s(actualIOCommitments, pos, randCommit[2], CODEWORD_BYTES, randCommit[3], CODEWORD_BYTES);
            pos += CODEWORD_BYTES;
            xorUI8s(actualIOCommitments, pos, CODEWORD_BYTES, (uint8_t*) &allLabels[2 * wire + 1 - b], CSEC_BYTES);
            
            // commit to perm bit
            pos += CODEWORD_BYTES;
            xorUI8s(actualIOCommitments, pos, randCommit[4], CODEWORD_BYTES, randCommit[5], CODEWORD_BYTES);
            pos += CODEWORD_BYTES;
            xorUI8s(actualIOCommitments, pos, CODEWORD_BYTES, &bit, 1);
        }

        send_channel.asyncSend(actualIOCommitments.data(), sizeof(uint8_t) * actualIOCommitments.size());
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
    xorUI8s(std::vector<uint8_t> &ret, int pos, uint8_t *vec1, int vec1_size, uint8_t *vec2, int vec2_size)
    {
        int vec_long_size = vec1_size;
        int vec_small_size = vec2_size;
        uint8_t *vec_long = vec1;
        uint8_t *vec_small = vec2;

        if (vec1_size < vec2_size)
        {
            vec_long_size = vec2_size;
            vec_long = vec2;
            vec_small_size = vec1_size;
            vec_small = vec1;
        }

//        if (ret.size() != vec_long_size)
//            ret.resize(vec_long_size);

        for (int i = 0; i < vec_long_size; i++)
        {
    //        print(ret.data(), vec_long_size);
            if (i < vec_small_size)
                ret[pos + i] = vec_long[i] ^ vec_small[i];
            else
                ret[pos + i] = vec_long[i];
        }
    }
    
    void
    xorUI8s(std::vector<uint8_t> &ret, int pos, int length, uint8_t *vec2, int vec2_size)
    {
        assert(length >= vec2_size);
        for (int i = 0; i < length; i++)
        {
    //        print(ret.data(), vec_long_size);
            if (i < vec2_size)
                ret[pos + i] = ret[pos + i] ^ vec2[i];
            else
                ret[pos + i] = ret[pos + i];
        }
    }
    
    void
    print(std::string desc, uint8_t *vec, int vec_num_entries)
    {
        std::ostringstream convert;
        convert << desc << "\t";
        for (int a = 0; a < vec_num_entries; a++) {
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