#include "XorHomomorphicCoordinator.h"

namespace xhCoordinator
{
    
    std::string chlIdStr(std::string name, osuCrypto::Role role, bool isSender, bool isFrom)
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
    
    XHCCoordinator::XHCCoordinator(int num_commits, osuCrypto::Role role)
    {
        int port = 28001;
        int num_execs = 1; // TODO: should be a separate parameter read from user
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
            randomSenderCommits( senders, role, num_execs, num_commits);
            
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
            randomReceiverCommits(receivers, exec_rnds, role, num_execs, num_commits);

            rec_ot_channel.close();            
        });

        futures[0].wait();
        futures[1].wait();

        end_point->stop();
        ios.stop();
    }
    
    void
    XHCCoordinator::randomSenderCommits(std::vector<SplitCommitSender> &senders, osuCrypto::Role role, int num_execs, int num_commits)
    {
        std::vector<osuCrypto::Channel*> send_channels;
        for (int e = 0; e < num_execs; ++e) {            
            send_channels.emplace_back(&end_point->addChannel(chlIdStr("commit_channel_" + std::to_string(e), role, true, true), chlIdStr("commit_channel_" + std::to_string(e), role, true, false)));
        }

//        ctpl::thread_pool thread_pool(std::thread::hardware_concurrency());

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
    XHCCoordinator::randomReceiverCommits(std::vector<SplitCommitReceiver> &receivers, std::vector<osuCrypto::PRNG> &exec_rnds, osuCrypto::Role role, int num_execs, int num_commits)
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
    XHCCoordinator::commitToInput(std::vector<bool> permBit, std::vector<osuCrypto::block> allInputLabels, Identity id, osuCrypto::Role role, osuCrypto::Channel &send_channel)
    {
        std::vector<uint8_t> inputCommitments(allInputLabels.size() * CODEWORD_BYTES);
        int pos = -1;
        for(uint64_t wire = 0; wire < allInputLabels.size(); wire++)
        {
            BYTEArrayVector tmp = send_commit_shares[0][0];// = getRandomCommitment(id, wire, true);
            pos = 2 * wire * CODEWORD_BYTES;
            xorUI8s(inputCommitments, pos, tmp[0], tmp.entry_size(), (uint8_t*) &allInputLabels[2 * wire], CSEC_BYTES);
            pos += CODEWORD_BYTES;
            xorUI8s(inputCommitments, pos, tmp[1], tmp.entry_size(), (uint8_t*) &allInputLabels[2 * wire + 1], CSEC_BYTES);
        }

        send_channel.asyncSend(inputCommitments.data(), sizeof(uint8_t) * inputCommitments.size());
    }
    
    void
    XHCCoordinator::receiveInputCommitments(Identity id, osuCrypto::Role role, int inputSize, osuCrypto::Channel &rec_channel)
    {
        std::vector<uint8_t> inputCommitments(inputSize * 2 * CODEWORD_BYTES);
        rec_channel.recv(inputCommitments.data(), sizeof(uint8_t) * inputCommitments.size());
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
                ret[i] = vec_long[i] ^ vec_small[i];
            else
                ret[i] = vec_long[i];
        }
    }
}