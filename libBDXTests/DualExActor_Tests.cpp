#include "DualExActor_Tests.h"
#include "Network/BtEndpoint.h"

#include "Common.h"
#include "Common/Defines.h"
#include "DualEx/DualExActor.h"
#include "Network/Channel.h"
#include "OTOracleReceiver.h"
#include "OTOracleSender.h"
#include "Common/Logger.h"
#include "DebugCircuits.h"

#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
//#include "MyAssert.h"
#include <array>

void DualExActor_BitAdder_Complete_Test_Impl()
{
	u64 numExe = 32,
		bucketSize = 4,
		numOpened = 4,
		psiSecParam = 40;

	Lg::setThreadName("Actor1");
	//InitDebugPrinting("..\\test.out");

	Circuit c = AdderCircuit(4);
	//NetworkManager netMgr0("127.0.0.1", 1212, 4, true);
	//NetworkManager netMgr1("127.0.0.1", 1212, 4, false);

	std::string name("ss");
	BtIOService ios(0);
	BtEndpoint ep0(ios, "localhost", 1212, true, name);
	BtEndpoint ep1(ios, "localhost", 1212, false, name);
	//Channel& recvChannel = ep1.addChannel(name, name);
	//Channel& senderChannel = ep0.addChannel(name, name);
	{

		PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
		PRNG prng1(prng0.get_block());
		BitVector input0(4);
		*input0.data() = 2;

		BitVector input1(4);
		*input1.data() = 3;

		BitVector expected(5);
		*expected.data() = 5;

		auto thrd = std::thread([&]() {
			Lg::setThreadName("Actor0");

			DualExActor actor0(c, Role::First, numExe, bucketSize, numOpened, psiSecParam, ep0);
			Timer timer;
			actor0.init(prng0, 4, 1, bucketSize, timer);

			for (u64 i = 0; i < numExe; ++i)
			{
				BitVector output = actor0.execute(i, prng0, input0, timer);

				for (u64 b = 0; b < expected.size(); ++b)
					if (output[b] != expected[b])
					{
						//Lg::out << input0 << " " << input1 << "  " << expected << Lg::endl;
						//Lg::out << "but got " << output << Lg::endl;
						throw UnitTestFail();
					}
			}
		});

		DualExActor actor1(c, Role::Second, numExe, bucketSize, numOpened, psiSecParam, ep1);
		Timer timer;
		actor1.init(prng1, 4, 1, bucketSize, timer);
		for (u64 i = 0; i < numExe; ++i)
		{

			BitVector output = actor1.execute(i, prng1, input1, timer);

			for (u64 b = 0; b < expected.size(); ++b)
				if (output[b] != expected[b])
					throw UnitTestFail();
		}


		thrd.join();
	}

	ep0.stop();
	ep1.stop();
	ios.stop();
}
void DualExActor_BitAdder_Concurrent_Test_Impl()
{
	u64 numExe = 32,
		bucketSize = 4,
		numOpened = 4,
		numConcurEvals = 4,
		psiSecParam = 40;

	Lg::setThreadName("Actor1");
	//InitDebugPrinting("..\\test.out");

	Circuit c = AdderCircuit(4);
	std::string name("ss");
	BtIOService ios(0);
	BtEndpoint ep0(ios, "localhost", 1212, true, name);
	BtEndpoint ep1(ios, "localhost", 1212, false, name);
	//Channel& recvChannel = ep1.addChannel(name, name);
	//Channel& senderChannel = ep0.addChannel(name, name);
	{
		PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
		PRNG prng1(prng0.get_block());
		BitVector input0(4);
		*input0.data() = 2;

		BitVector input1(4);
		*input1.data() = 3;

		BitVector expected(5);
		*expected.data() = 5;

		auto thrd = std::thread([&]() {
			Lg::setThreadName("Actor0");

			DualExActor actor0(c, Role::First, numExe, bucketSize, numOpened, psiSecParam, ep0);
			Timer timer;
			actor0.init(prng0, 4, numConcurEvals, bucketSize, timer);
			std::vector<std::thread> evalThreads(numConcurEvals);

			for (u64 j = 0; j < numConcurEvals; ++j)
			{
				block seed = prng0.get_block();
				evalThreads[j] = std::thread([&, j, seed]() {
					Timer timerj;
					PRNG prng(seed);
					for (u64 i = j; i < numExe; i += numConcurEvals)
					{

						BitVector output = actor0.execute(i, prng, input0, timerj);

						for (u64 b = 0; b < expected.size(); ++b)
							if (output[b] != expected[b])
								throw UnitTestFail();
					}
				});
			}
			for (auto& evalThrd : evalThreads)
				evalThrd.join();
		});

		DualExActor actor1(c, Role::Second, numExe, bucketSize, numOpened, psiSecParam, ep1);
		Timer timer;
		actor1.init(prng1, 4, numConcurEvals, bucketSize, timer);

		std::vector<std::thread> evalThreads(numConcurEvals);

		for (u64 j = 0; j < numConcurEvals; ++j)
		{
			block seed = prng1.get_block();
			evalThreads[j] = std::thread([&actor1, j, seed, numExe, numConcurEvals, &input1, &expected]() {
				Timer timerj;
				PRNG prng(seed);
				for (u64 i = j; i < numExe; i += numConcurEvals)
				{
					BitVector output = actor1.execute(i, prng, input1, timerj);

					for (u64 b = 0; b < expected.size(); ++b)
						if (output[b] != expected[b])
							throw UnitTestFail();
				}
			});

		}
		for (auto& evalThrd : evalThreads)
			evalThrd.join();

		thrd.join();
	}

	ep0.stop();
	ep1.stop();
	ios.stop();

}
