
#include "batch2pc.h"


namespace batchRam
{

    
    void
    Batch2PC::read_circuit(osuCrypto::Circuit &cir, std::string file)
    {
        std::cout << "reading circuit" << std::endl;
        {
            std::fstream fStrm(file);
            if (fStrm.is_open() == false)
            {
//                boost::filesystem::path getcwd(boost::filesystem::current_path());
//                std::cout << "Current path is: " << getcwd << std::endl;
                std::cout << "failed to open circuit file: " << file << std::endl;

                throw std::runtime_error("");
            }

            cir.readBris(fStrm);
        }

        std::cout << "circuit inputs " << cir.Inputs()[0] << " " << cir.Inputs()[1] << std::endl;
    }

    Batch2PC::Batch2PC(std::string circ_path, osuCrypto::Role role, xhCoordinator::XHCCoordinator &xhcCoordinator, std::string name, int id, int numExec, int bucketSize, int numOpened, int psiSecParam, int numConcurrentSetups, int numConcurrentEvals, int numThreadsPerEval)
    :
    computationId(id),
    mRole(role)
    {
        read_circuit(cir, circ_path);
        
        
        std::cout << "     --> Net init" << std::endl;
	ios = new osuCrypto::BtIOService(0);
        netMgr = new osuCrypto::BtEndpoint(*ios, "127.0.0.1", 1212 + id + 1, mRole, "ss");
        actor = new osuCrypto::DualExActor(cir, mRole, numExec, bucketSize, numOpened, psiSecParam, xhcCoordinator, computationId, *netMgr);
	prng = new osuCrypto::PRNG(_mm_set_epi64x(0, mRole));

        std::cout << "     --> 2PC offline" << std::endl;
	actor->init(*prng, numConcurrentSetups, numConcurrentEvals, numThreadsPerEval, timer);
    }
    
    void
    Batch2PC::setDirectInputWires(int bucketIdx, int lengths[2])
    {

    }

    void
    Batch2PC::prepareInputs(int bucketIdx)
    {

    }

    void
    Batch2PC::initEvaluate()
    {
        std::cout << "     --> 2PC online" << std::endl;
	// do one without the timing to sync the two parties...
	input = osuCrypto::BitVector(cir.Inputs()[mRole]);

	osuCrypto::u64 sleepTime = 100;
	std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
	//actor.execute(0, input, timer);
	//--numExec;        
        
        std::cout << "     -->  exec" << std::endl;
        osuCrypto::block seed = prng->get<osuCrypto::block>();
        prng2 = new osuCrypto::PRNG(seed);
    }
    
    void
    Batch2PC::evaluate(osuCrypto::u64 bucketIdx, std::vector<osuCrypto::block> &garbledOutputs)
    { 
        actor->execute(bucketIdx, *prng2, input, timer, garbledOutputs);
    }
    
    void
    Batch2PC::cleanup() {
        std::cout << "     -->  close" << std::endl;
        actor->close();
        netMgr->stop();
        ios->stop();
    }


    int
    Batch2PC::getWiresAsPlainNumber(int bucketIdx, int length)
    {
        return -1;
    }

    std::vector<std::array<xhCoordinator::XorHomomorphicCommit, 2> >
    Batch2PC::getXHCommitments(int bktIdx, std::vector<int> wireIndexes)
    {
        return std::vector<std::array<xhCoordinator::XorHomomorphicCommit, 2> >();            
    }

    std::vector<xhCoordinator::XorHomomorphicCommit>
    Batch2PC::getXHPointNPermuteCommitments(int bktIdx, std::vector<int> wireIndexes)
    {
        return std::vector<xhCoordinator::XorHomomorphicCommit>();
    }

    std::vector<std::pair<osuCrypto::block, std::array<xhCoordinator::XorHomomorphicCommit, 2> > >
    Batch2PC::getWireAndXHCommitments(int bktIdx, std::vector<int> wireIndexes)
    {
        return std::vector<std::pair<osuCrypto::block, std::array<xhCoordinator::XorHomomorphicCommit, 2> > >();
    }

    void
    Batch2PC::setXHCInputs(int bktIdx, std::vector<int> wireIndexes, std::vector<osuCrypto::block> garbledInputs)
    {

    }

    std::string
    Batch2PC::getName()
    {
        return "";
    }
    
    Identity
    Batch2PC::getBucketHeadId(int bucketIdx)
    {
        return actor->getBucketHeadId(bucketIdx);
    }
    
    std::vector<int>
    Batch2PC::getRelativeOutputWireIndexes()
    {
        std::vector<int> fixme(128);
        for (int i = 0; i < fixme.size(); i++) {
            fixme[i] = i;
        }
        
        return fixme;
    }
    
    std::vector<int>
    Batch2PC::getRelativeInputWireIndexes()
    {
        std::vector<int> fixme(128);
        for (int i = 0; i < fixme.size(); i++) {
            fixme[i] = i;
        }
        
        return fixme;
    }

}