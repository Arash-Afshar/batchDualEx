#include "mains.h"

#include "split-commit/split-commit-rec.h"

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


int main(int argc, const char* argv[]) {
    int num_commits, num_execs, port;
    std::string ip_address;

    num_commits = 10000;
    num_execs = 1;
    ip_address = "localhost";
    port = 28001;


    osuCrypto::BtIOService ios(0);
    osuCrypto::BtEndpoint rec_end_point(ios, ip_address, port, false, "ep");
    osuCrypto::Channel& rec_ot_channel = rec_end_point.addChannel("ot_channel", "ot_channel");

    osuCrypto::PRNG rnd;
    rnd.SetSeed(load_block(constant_seeds[1].data()));
    SplitCommitReceiver base_receiver;
    base_receiver.SetMsgBitSize(128);

    //Seed OTs
    auto seed_ot_begin = GET_TIME();

    base_receiver.ComputeAndSetSeedOTs(rnd, rec_ot_channel);
    rec_ot_channel.close();

    auto seed_ot_end = GET_TIME();

    std::vector<osuCrypto::Channel*> rec_channels;
    for (int e = 0; e < num_execs; ++e) {
        rec_channels.emplace_back(&rec_end_point.addChannel("commit_channel_" + std::to_string(e), "commit_channel_" + std::to_string(e)));
    }

    std::vector<SplitCommitReceiver> receivers(num_execs);
    std::vector<osuCrypto::PRNG> exec_rnds(num_execs);

    base_receiver.GetCloneReceivers(num_execs, rnd, receivers, exec_rnds);

    ctpl::thread_pool thread_pool(std::thread::hardware_concurrency());

    std::vector<std::future<void>> futures(num_execs);
    uint32_t exec_num_commits = CEIL_DIVIDE(num_commits, num_execs);

    auto commit_begin = GET_TIME();
    std::vector<BYTEArrayVector> rec_commit_shares(num_execs, BYTEArrayVector(exec_num_commits, CODEWORD_BYTES));
    for (int e = 0; e < num_execs; ++e) {

        futures[e] = thread_pool.push([&rec_end_point, &exec_rnds, &receivers, &rec_commit_shares, &rec_channels, exec_num_commits, e](int id) {

            receivers[e].Commit(rec_commit_shares[e], exec_rnds[e], *rec_channels[e]);

        });
    }

    for (std::future<void>& r : futures) {
        r.wait();
    }


    std::vector<uint8_t> realCommit(CODEWORD_BYTES);
    rec_channels[0]->recv(realCommit.data(), realCommit.size());
    print("realCommit:", realCommit.data(), realCommit.size());


    auto commit_end = GET_TIME();

    auto decommit_begin = GET_TIME();
            BYTEArrayVector tmp(exec_num_commits, CSEC_BYTES);
    for (int e = 0; e < num_execs; ++e) {

        futures[e] = thread_pool.push([&rec_end_point, &exec_rnds, &receivers, &rec_commit_shares, &rec_channels, exec_num_commits, e, &tmp](int id) {

            receivers[e].Decommit(rec_commit_shares[e], tmp, *rec_channels[e]);

        });
    }

    for (std::future<void>& r : futures) {
        r.wait();
    }
    
    xorUI8s(realCommit, realCommit.data(), realCommit.size(), tmp[0], tmp.entry_size());
    int LABEL_SIZE = sizeof(osuCrypto::block);
    uint8_t label[16];

    for (int i = 0; i < 16; i++) {
        label[i] = realCommit[i];
    }
    print("opened:\t", label, LABEL_SIZE);

    
    auto decommit_end = GET_TIME();

//    auto batch_decommit_begin = GET_TIME();
//    for (int e = 0; e < num_execs; ++e) {
//
//        futures[e] = thread_pool.push([&rec_end_point, &exec_rnds, &receivers, &rec_commit_shares, &rec_channels, exec_num_commits, e](int id) {
//
//            BYTEArrayVector tmp(exec_num_commits, CSEC_BYTES);
//            receivers[e].BatchDecommit(rec_commit_shares[e], tmp, exec_rnds[e], *rec_channels[e]);
//
//        });
//    }
//
//    for (std::future<void>& r : futures) {
//        r.wait();
//    }
//
//    auto batch_decommit_end = GET_TIME();

    for (int e = 0; e < num_execs; ++e) {
        rec_channels[e]->close();
    }

    rec_end_point.stop();
    ios.stop();

//    uint64_t seed_ot_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(seed_ot_end - seed_ot_begin).count();
//    uint64_t commit_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(commit_end - commit_begin).count();
//    uint64_t decommit_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(decommit_end - decommit_begin).count();
//    uint64_t batch_decommit_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(batch_decommit_end - batch_decommit_begin).count();
//
//    std::cout << "===== Timings for receiver doing " << num_commits << " random commits using " << num_execs << " parallel execs " << std::endl;
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
