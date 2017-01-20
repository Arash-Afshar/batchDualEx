#include "mains.h"

#include "split-commit/split-commit-snd.h"

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

void xorUI8s(std::vector<uint8_t> &ret, uint8_t *vec1, int vec1_size, uint8_t *vec2, int vec2_size)
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
    
    
    ret.resize(vec_long_size);
    
    for (int i = 0; i < vec_long_size; i++)
    {
//        print(ret.data(), vec_long_size);
        if (i < vec_small_size)
            ret[i] = vec_long[i] ^ vec_small[i];
        else
            ret[i] = vec_long[i];
    }
}

int main(int argc, const char* argv[])
{

    int num_commits, num_execs, port;
    std::string ip_address;

    num_commits = 10000;
    num_execs = 1;
    ip_address = "localhost";
    port = 28001;


    osuCrypto::BtIOService ios(0);
    osuCrypto::BtEndpoint send_end_point(ios, ip_address, port, true, "ep");
    osuCrypto::Channel& send_ot_channel = send_end_point.addChannel("ot_channel", "ot_channel");

    osuCrypto::PRNG rnd;
    rnd.SetSeed(load_block(constant_seeds[0].data()));

    SplitCommitSender base_sender;
    base_sender.SetMsgBitSize(128);

    //Seed OTs
    auto seed_ot_begin = GET_TIME();

    base_sender.ComputeAndSetSeedOTs(rnd, send_ot_channel);
    send_ot_channel.close();

    auto seed_ot_end = GET_TIME();
    int e = 0;
    osuCrypto::Channel* send_channel = &send_end_point.addChannel("commit_channel_" + std::to_string(e), "commit_channel_" + std::to_string(e));

    std::vector<SplitCommitSender> senders(num_execs);
    base_sender.GetCloneSenders(num_execs, senders);

    ctpl::thread_pool thread_pool(std::thread::hardware_concurrency());

    std::vector<std::future<void> > futures(num_execs);
    uint32_t exec_num_commits = CEIL_DIVIDE(num_commits, num_execs);

    auto commit_begin = GET_TIME();
    std::vector<std::array<BYTEArrayVector, 2> > send_commit_shares(num_execs,{
        BYTEArrayVector(exec_num_commits, CODEWORD_BYTES),
        BYTEArrayVector(exec_num_commits, CODEWORD_BYTES)
    });


    futures[e] = thread_pool.push([&send_end_point, &senders, &send_commit_shares, &send_channel, exec_num_commits, e](int id) {
        senders[e].Commit(send_commit_shares[e], *send_channel);
    });


    for (std::future<void>& r : futures) {
        r.wait();
    }

//    std::cout << "after:" << std::endl;
    osuCrypto::PRNG prng(_mm_set_epi64x(0, 0));
    osuCrypto::block label = prng.get<osuCrypto::block>();
    uint8_t *lableBytes = (uint8_t*) &label;
    int LABEL_SIZE = sizeof (osuCrypto::block) / sizeof (uint8_t);
    
    std::vector<uint8_t> realCommit;
    xorUI8s(realCommit, send_commit_shares[e][1][0], send_commit_shares[e][1].entry_size(), send_commit_shares[e][0][0], send_commit_shares[e][0].entry_size());
    print("r[0]^r[1]:", realCommit.data(), realCommit.size());
    xorUI8s(realCommit, realCommit.data(), realCommit.size(), lableBytes, LABEL_SIZE);
    
    print("realCommit:", realCommit.data(), realCommit.size());
    print("label:\t", lableBytes, LABEL_SIZE);
    print("rand[0]:", send_commit_shares[e][0][0], send_commit_shares[e][0].entry_size());
    print("rand[1]:", send_commit_shares[e][1][0], send_commit_shares[e][1].entry_size());
    std::cout << "-------------" << std::endl;
    
    send_channel->send(realCommit.data(), realCommit.size());
    

//    std::cout << sizeof (glableByte) / sizeof (uint8_t) << ","  << send_commit_shares[e][0].entry_size() << std::endl;
//    print(glableByte, sizeof (glableByte) / sizeof (uint8_t));
//    std::vector<uint8_t> glableByte3;
//    xorUI8s(glableByte3, glableByte, 8, send_commit_shares[e][0][0], send_commit_shares[e][0].entry_size());
//    print(send_commit_shares[e][0][0], send_commit_shares[e][0].entry_size());
//    print(glableByte3.data(), glableByte3.size());
//    
//    std::cout <<  glableByte3.size() << "," << sizeof (glableByte3.data()) / sizeof (uint8_t) << std::endl;
    

    auto commit_end = GET_TIME();

    auto decommit_begin = GET_TIME();
    futures[e] = thread_pool.push([&send_end_point, &senders, &send_commit_shares, &send_channel, exec_num_commits, e](int id) {
        senders[e].Decommit(send_commit_shares[e], *send_channel);

    });


    for (std::future<void>& r : futures) {
        r.wait();
    }

    auto decommit_end = GET_TIME();

//    auto batch_decommit_begin = GET_TIME();
//    futures[e] = thread_pool.push([&send_end_point, &senders, &send_commit_shares, &send_channel, exec_num_commits, e](int id) {
//
//
//        senders[e].BatchDecommit(send_commit_shares[e], *send_channel);
//
//    });
//
//
//    for (std::future<void>& r : futures) {
//        r.wait();
//    }
//
//    auto batch_decommit_end = GET_TIME();

    send_channel->close();


    send_end_point.stop();
    ios.stop();

//    uint64_t seed_ot_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(seed_ot_end - seed_ot_begin).count();
//    uint64_t commit_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(commit_end - commit_begin).count();
//    uint64_t decommit_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(decommit_end - decommit_begin).count();
//    uint64_t batch_decommit_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(batch_decommit_end - batch_decommit_begin).count();

//    std::cout << "===== Timings for sender doing " << num_commits << " random commits using " << num_execs << " parallel execs " << std::endl;
//
//    std::cout << "OT ms: " << (double) seed_ot_time_nano / 1000000 << std::endl;
//    std::cout << "Amortized OT ms: " << (double) seed_ot_time_nano / num_commits / 1000000 << std::endl;
//    std::cout << "Commit us (with OT): " << (double) (commit_time_nano + seed_ot_time_nano) / num_commits / 1000 << std::endl;
//    std::cout << "Commit us: " << (double) commit_time_nano / num_commits / 1000 << std::endl;
//    std::cout << "Commit total ms: " << (double) (commit_time_nano + seed_ot_time_nano) / 1000000 << std::endl;
//    std::cout << "Decommit us: " << (double) decommit_time_nano / num_commits / 1000 << std::endl;
//    std::cout << "Decommit total ms: " << (double) decommit_time_nano / 1000000 << std::endl;
//    std::cout << "BatchDecommit us: " << (double) batch_decommit_time_nano / num_commits / 1000 << std::endl;
//    std::cout << "BatchDecommit total ms: " << (double) batch_decommit_time_nano / 1000000 << std::endl;

    return 0;
}