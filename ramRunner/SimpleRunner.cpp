//#include "Common.h"
//#include "Common/Timer.h"
//#include "Common/Exceptions.h"
//#include "Common/Defines.h"
#include "cryptoTools/Common/Log.h"


#include "DualEx/DualExActor.h"

#include "cryptoTools/Network/Channel.h"
#include "cryptoTools/Network/BtEndpoint.h"

//#include "boost/asio.hpp"
//#include "boost/filesystem.hpp"

//#include <vector>
//#include <thread>
//#include <functional>
//#include <cassert>
//#include <chrono>
//#include <stdlib.h>
//#include <sstream>
//#include <fstream>
#include <iostream> 


//#include "OT/OTExtension.h"
//#include "Circuit/Circuit.h"
//#include "DebugCircuits.h"

//#include "../FrontEnd/UnitTests.h"
#include "libRAM/coordinator.h"

#include "../FrontEnd/ezOptionParser.h" 
using namespace ez;

using namespace osuCrypto;
using namespace std;

void commandLineMain(int argc, const char** argv);
void setup_arg_parser(int argc, const char** argv, std::string &hostname,  std::string &file,
	u64 &portnum, u64 &numExec, u64 &bucketSize, u64 &numOpened, u64 &numConcurrentSetups, u64 &numConcurrentEvals, u64 &numThreadsPerEval, u64 &psiSecParam,
	Role &role, Timer &timer);
//void read_circuit(Circuit &cir, std::string file);

int main(int argc, const char** argv)
{
    commandLineMain(argc, argv);
}

void setup_arg_parser(int argc, const char** argv,
        std::string &hostname,  std::string &file,
	u64 &portnum, u64 &numExec, u64 &bucketSize, u64 &numOpened, u64 &numConcurrentSetups, u64 &numConcurrentEvals, u64 &numThreadsPerEval, u64 &psiSecParam,
	Role &role, Timer &timer,
        int &memoryLength, int &memoryCellLength, int &instructionLength){
    	ezOptionParser opt;
	opt.add(
		"",
		0,
		1,
		0,
		"run unit tests, using this as the directory for unit tests data folder",
		"-u",
		"--unittest");

	opt.add(
		"", // Default.
		0, // Required?
		1, // Number of args expected.
		0, // Delimiter if expecting multiple args.
		"This player's role, 0/1 (required).", // Help description.
		"-r", // Flag token.
		"--role" // Flag token.
		);

	opt.add(
		"5000", // Default.
		0, // Required?
		1, // Number of args expected.
		0, // Delimiter if expecting multiple args.
		"Base port number used by Server.x (default: 5000).", // Help description.
		"-p", // Flag token.
		"--portnum" // Flag token.
		);

	opt.add(
		"localhost", // Default.
		0, // Required?
		1, // Number of args expected.
		0, // Delimiter if expecting multiple args.
		"Host name that Server.x is running on (default: localhost).", // Help description.
		"-h", // Flag token.
		"--hostname" // Flag token.
		);

	opt.add(
		"128",
		0,
		1,
		0,
		"Number of executions to run (default: 128).",
		"-n",
		"--nExec"
		);

	opt.add(
		"4",
		0,
		1,
		0,
		"bucket size (default: 4).",
		"-b",
		"--bcktSize"
		);


	opt.add(
		"0",
		0,
		1,
		0,
		"Number of opened circuits(default: 100).",
		"-o",
		"--open"
		);


	opt.add(
		"./circuits/frigatedef/base/",
		0,
		1,
		0,
		"Circuit description file",
		"-f",
		"--file");

	opt.add(
		"",
		0,
		0,
		0,
		"perform network ping at the start",
		"-i",
		"--ping");

	opt.add(
		"4",
		0,
		1,
		0,
		"number of concurrent setup phases. (4x threads) and defaults to 4",
		"-s",
		"--setupConcurrently");

	opt.add(
		"1",
		0,
		1,
		0,
		"denotes the number of concurrent evaluations. defaults to 1 (sequential)",
		"-e",
		"--evalConcurrently");

	opt.add(
		"",
		0,
		1,
		0,
		"denotes the number of circuit threads per evaluations. defaults to bucket size",
		"-c",
		"--circuitThreads");

	opt.add(
		"40",
		0,
		1,
		0,
		"statistical security param",
		"-k",
		"--statisticalK");

	opt.add(
		"16",
		0,
		1,
		0,
		"RAM length",
		"-m",
		"--memoryLength");

	opt.add(
		"16",
		0,
		1,
		0,
		"RAM element length",
		"-z",
		"--memoryCellLength");

	opt.add(
		"4",
		0,
		1,
		0,
		"Instruction length",
		"-w",
		"--instructionLength");

	opt.parse(argc, argv);

	int temp;
	opt.get("-r")->getInt(temp); role = (Role)temp;
	opt.get("-n")->getInt(temp); numExec = static_cast<u64>(temp);
	opt.get("-b")->getInt(temp); bucketSize = static_cast<u64>(temp);
	opt.get("-o")->getInt(temp); numOpened = static_cast<u64>(temp);
	opt.get("-p")->getInt(temp); portnum = static_cast<u64>(temp);
	opt.get("-s")->getInt(temp); numConcurrentSetups = static_cast<u64>(temp);
	opt.get("-e")->getInt(temp); numConcurrentEvals = static_cast<u64>(temp);
	opt.get("-k")->getInt(temp); psiSecParam = static_cast<u64>(temp);
	opt.get("-h")->getString(hostname);
	opt.get("-f")->getString(file);

        opt.get("-m")->getInt(memoryLength);
	opt.get("-z")->getInt(memoryCellLength);
	opt.get("-w")->getInt(instructionLength);


	if (opt.get("-c")->isSet)
	{
		opt.get("-c")->getInt(temp); numThreadsPerEval = static_cast<u64>(temp);
	}
	else
	{
		numThreadsPerEval = bucketSize;
	}

	std::cout << "role: " << (int)role
		<< "  numExe:" << numExec
		<< "  bucketSize:" << bucketSize
		<< "  numOpen:" << numOpened
		<< "  ConcurrentSetups:" << numConcurrentSetups
		<< "  ConcurrentEvals:" << numConcurrentEvals
		<< "  numThreadsPerEval:" << numThreadsPerEval << std::endl;


        if (opt.get("-r")->isSet == false)
	{
            cout << "must set -r" << endl;
            exit(1);
	}

}

//void read_circuit(Circuit &cir, std::string file)
//{
//	std::cout << "reading circuit" << std::endl;
//
//	{
//		std::fstream fStrm(file);
//		if (fStrm.is_open() == false)
//		{
//			boost::filesystem::path getcwd(boost::filesystem::current_path());
//			std::cout << "Current path is: " << getcwd << std::endl;
//			std::cout << "failed to open circuit file: " << file << std::endl;
//
//			throw std::runtime_error("");
//		}
//
//		cir.readBris(fStrm);
//	}
//
//	std::cout << "circuit inputs " << cir.Inputs()[0] << " " << cir.Inputs()[1] << std::endl;
//}

//class Coordinator
//{
//public:
//    Endpoint &mNetMgr;
//    Coordinator(Endpoint &netMgr):
//    mNetMgr(netMgr)
//    {
//        
//    }
//    
//    void sendInputs(int i, int j, int inputSize){
//        Channel& chl = mNetMgr.addChannel("CoordinatorSND" + ToString(j) + "_" + ToString(i), "CoordinatorRCV" + ToString(j) + "_" + ToString(i));
////        std::vector<block>& allLabels;
////        chl.recv(allLabels.data(), inputSize * sizeof(block));
////        timer.setTimePoint("myInputs");
//        chl.close();
//    }
//    
//    std::thread spawnThread(int i, int j, int inputSize){
//        return std::thread([this, i,j, inputSize]()
//        {
//            sendInputs(i,j, inputSize);
//        });
//    }
//
//};

void commandLineMain(int argc, const char** argv)
{
//	block b;
	Timer timer;
//	Circuit cir;
//        int actor_count = 2;

//	AES128::Key key;
//	AES128::EncKeyGen(b, key);
//	AES128::EcbEncBlock(key, b, b);
        
        int memoryLength, memoryCellLength, instructionLength;
	std::string hostname, circ_path_prefix;
	u64 portnum, numExec, bucketSize, numOpened, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval, psiSecParam;
	Role role;
         
        setup_arg_parser(argc, argv, hostname, circ_path_prefix, portnum, numExec, bucketSize, numOpened, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval, psiSecParam, role, timer, memoryLength, memoryCellLength, instructionLength);
	
        BtIOService ios(0);
        BtEndpoint coordinatorNetManager(ios, "127.0.0.1", 1212, role, "ss");
        Channel& chl = coordinatorNetManager.addChannel("Coordinator" + ToString(role), "Coordinator" + ToString(1 - role));
        
        batchRam::Coordinator coordinator(chl, role, circ_path_prefix, memoryLength, memoryCellLength, instructionLength, numExec, bucketSize, numOpened, psiSecParam, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval);
        
        
//	libBDX::Circuit cir;
//        read_circuit(cir, circ_path);
//
//        BtEndpoint* netMgrs[actor_count];
//        DualExActor* actors[actor_count];
//        for(int i = 0; i < actor_count; i++)
//        {
//            netMgrs[i] = new BtEndpoint(ios, "127.0.0.1", 1212 + i, role, "ss");
//            actors[i] = new DualExActor(cir, role, numExec, bucketSize, numOpened, psiSecParam, *netMgrs[i]);
//        }
//	//NetworkManager netMgr(hostname, portnum, 6, role);
//
//	PRNG prng(_mm_set_epi64x(0, role));
//
//	std::cout << "Initializing..." << std::endl;
//	actors[0]->init(prng, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval, timer);
//	actors[1]->init(prng, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval, timer);
//        
//	std::cout << "exec" << std::endl;
//	// do one without the timing to sync the two parties...
//	BitVector input(cir.Inputs()[role]);
//
//	u64 sleepTime = 100;
//	std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
//	//actor.execute(0, input, timer);
//	//--numExec;        
//        
//        std::vector<std::thread> threads0(numConcurrentEvals * numThreadsPerEval);
//        std::vector<std::thread> threads1(numConcurrentEvals * numThreadsPerEval);
//
//        Coordinator coordinator0(*netMgrs[0]);
//        Coordinator coordinator1(*netMgrs[1]);
//        
//        for (u64 j = 0; j < numConcurrentEvals; ++j)
//        {
//            for (u64 i = 0; i < numThreadsPerEval; ++i)
//            {
//                threads0[j * numThreadsPerEval + i] = coordinator0.spawnThread(i, j, cir.Inputs()[0]);
//                threads1[j * numThreadsPerEval + i] = coordinator1.spawnThread(i, j, cir.Inputs()[1]);
//            }
//        }
//
//        block seed = prng.get_block();
//        PRNG prng2(seed);
//        for (u64 i = 0; i < static_cast<u64>(numExec); i += numConcurrentEvals)
//        {
//            for(int k = 0; k < actor_count; k++)
//            {
//                actors[k]->execute(i, prng2, input, timer);
//            }
//        }
//        
//        for (u64 j = 0; j < numConcurrentEvals; ++j)
//        {
//            for (u64 i = 0; i < numThreadsPerEval; ++i)
//            {
//                threads0[j * numThreadsPerEval + i].join();
//                threads1[j * numThreadsPerEval + i].join();
//            }
//        }
//
//        for(int i = 0; i < actor_count; i++)
//        {
//            actors[i]->close();
//            netMgrs[i]->stop();
//        }
        
        coordinator.cleanup();
        coordinatorNetManager.stop();
	ios.stop();
	return;
}
