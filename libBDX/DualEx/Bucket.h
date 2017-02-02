#pragma once
#include <vector>
#include "cryptoTools/Common/BitVector.h"
#include "CircuitPackage.h"
#include "Circuit/KProbeResistant.h"
#include "PSI/PSIReceiver.h"
#include "PSI/PSISender.h"
#include "PSI/AsyncPsiReceiver.h"
#include "PSI/AsyncPsiSender.h"
#include <future>
#include "libRAM/XorHomomorphicCoordinator.h"
#include "libRAM/identity.h"
#include "cryptoTools/Common/Timer.h"

//#define DUALEX_DEBUG
//#define ASYNC_PSI 
#define OFFLINE_KPROBE

namespace osuCrypto
{
	class Bucket
	{
            
	public:
		Bucket();
		~Bucket();
                
		void initRecv(
			const Circuit& cir,
			Channel& chl,
			KProbeMatrix& theirKProbe,
			u64 bucketSize,
			u64 psiSecParam,
			std::vector<u64>::iterator& cirIdxIter,
			std::vector<CommCircuitPackage>& circuits,
            BDX_OTExtSender  & otSend,
			PRNG& prng,
			u64& otIdx,
			Role role);


		void initSend(
			const Circuit& cir, 
			Channel& chl,
			u64 bucketSize,
			u64 psiSecParam,
			Role role,
			std::vector<CircuitPackage>& circuits, 
			std::vector<u64>::iterator& cirIdxIter,
			PRNG& prng,
			const KProbeMatrix& theirKProbe,
			const KProbeMatrix& myKprobe,
			BDX_OTExtReceiver & otRecv,
			u64& otIdx,
			const std::vector<block>& indexArray);
			

		void initKProbeInputRecv(
			const Circuit& cir,
			Channel& chl,
			KProbeMatrix& myKProbe,
			PRNG& prng,
			Role role);

		void initKProbeInputSend(const Circuit & cir, Channel & chl, KProbeMatrix & theirKProbe, PRNG & prng, Role role, const std::vector<block>& indexArray);


		void evaluate(
			const Circuit&  cir,
			const KProbeMatrix& theirKprobe,
			const KProbeMatrix& myKprobe,
			PRNG& prng, 
			Channel& chl,
			Role role,
			const BitVector& input,
			std::vector<std::vector<block>>& labels, 
			osuCrypto::Timer& timer);

		void sendCircuitInputs(
			const Circuit&  cir,
			const BitVector & input,
			Role role,
			Channel& chl,
                        xhCoordinator::XHCCoordinator &xhcCoordinator,
			u64 circuitOffset,
			u64 circuitStep);

		void openTranslation(const Circuit& cir,  
			Channel& chl);


		void checkTranslation(const Circuit& cir,
			u64 cirIdx, 
			std::vector<block>& wireBuff,
			Role role);
		void Clear();
                
                Identity getHeadId();

        // returns the bucket-wide (head) garbled output of 
        // the evaluated garbled circuits
        // @ dest [out]: The location that the wire labels should be stored.
        void getGarbledOutput(
            const ArrayView<block>& dest);

        // returns the bucket-wide (head) garbled output of 
        // the Gerbled garbled circuits.
        // @ dest [out]: THe location that the putput wire labels should be stored
        void getGarbledOutput(
            const ArrayView<std::array<block,2>>& dest);

        // returns the circuitIdx'th garbled output for the evaluated circuits.
        // @ circuitIdx [in]: The index of the circuit that the garbled output
        //      labels should be taken. 
        // @ dest [out]: The location that the garbled output labels should be stored
        void getGarbledOutput(
            u64 circuitIdx, 
            const ArrayView<block>& dest);

        // returns the circuitIdx'th garbled output labels that were generated locally.
        // @ circuitIdx [in]: The index of the circuit that the garbled output
        //      labels should be taken. 
        // @ dest [out]: The location that the garbled output labels should be stored.
        // @ freeXorOffset [out]: The location this circuits free xor wire offset should be stored.
        void getGarbledOutput(
            u64 circuitIdx, 
            const ArrayView<block>& dest,
            block& freeXorOffset);

        // sets the garbled inputs on the circuitIdx'th evaluation circuit.
        //     e.g. evalCircuit[circuitIdx].inputLabels[wireIdx[i]] = src[i]; for all i.
        // @ wireIdxs [in]: The indexes of the input labels that should be set.
        //     Note: parameter will be moved.
        // @ src [in]: The wire labels that should be used. One set for each circuit
        //     Note: parameter will be moved.
        void setGarbledInput(
            std::vector<u64>&& wireIdxs,
            std::vector<std::vector<block>>&& src);



        u32 mPSIIdxHead;
        std::vector<BitVector> mOutputs;

		BitVector mTheirInputCorrection;
		std::unique_ptr<BitVector> mInputCorrectionString;

		std::promise<const BitVector*> mInputPromise;
		std::shared_future<const BitVector*> mInputFuture;

		//std::promise<std::vector<block>*> mPsiInputsPromise;
		//std::future<std::vector<block>*> mPsiInputsFuture;

		std::promise<void> mTheirDeltasProm, mTheirInputCorrectionPromise, mMyCircuitsBucketedProm, mTranslationCheckDoneProm;// , mPSIInputCommittedProm;
		std::shared_future<void> mTheirDeltasFuture, mTheirInputCorrectionFuture, mMyCircuitsBucketedFuture, mTranslationCheckDoneFuture;// , mPSIInputCommittedFuture;

		std::vector<std::promise<void>> mTheirInputLabelsProm;
		std::vector<std::future<void>> mTheirInputLabelsFuture;

		std::vector<std::promise<block>> mPsiInputPromise;
		std::vector<std::shared_future<block>> mPsiInputFuture;
		std::vector<std::vector<block>> mMyDecodedKProbeLabels, mTheirInputOffsets;
 
		std::promise<std::vector<block>*> mTheirOutputLabelsProm;
		std::shared_future<std::vector<block>*> mTheirOutputLabelsFuture;
		 
		std::promise<BitVector*> mOutputProm;
		std::future<BitVector*> mOutputFuture;

		std::atomic<i32>  mTransCheckRemaining, mOutputMissCount;
		std::vector<block> mPSIInputBlocks;

		block mOutputLabelSeed;

		block evalCircuit(
			u64 i,
			const Circuit&  cir,
			PRNG& prng,
			const KProbeMatrix& myKprobe,
			std::vector<block>& labels,
			Channel& chl,
                        xhCoordinator::XHCCoordinator &xhcCoordinator,
#ifdef ADAPTIVE_SECURE
			std::vector<block> adaptiveSecureTableMasks,
			const std::vector<block>& indexArray,
#endif
			Role role,
			Timer& timer);

		/// a matrix of m rows each of size (bucketSize-1) which holds the xor differences between the random ot choice bits of their first circuit and the i'th circuit. 
		std::vector<BitVector> mTheirPermutes; //mTheirDeltas;
		/// the circuits and related material that they generated in the setup phase. we will evaluate these.
		std::vector<CommCircuitPackage*> mTheirCircuits;
		/// the circuits and related material that we generated in the set up phase. they will evaluate these.
		std::vector<CircuitPackage*> mMyCircuits;
		/// these values allow us to translate the output labels we got by evaluating to the common output labels that their chose for their bucket
		std::vector<std::vector<std::array<block, 2>>> mTranlation;
		/// these are the common output labels that we chose for this bucket. they will use their translation values to map to these values.
		std::vector<std::array<block, 2>> mCommonOutput;

        // these are the evaluated common output encoding (head) that the PSI said was correct.
        std::vector<block> mCommonEvalOutput;
        // these are the evaluated output encoding that the PSI said was correct.
        std::vector<std::vector<block>> mEvalOutput;

        //std::vector<std::pair<std::vector<u64>>

        // the circuit input wire indexes that shouls be copy to
        std::vector<u64> mCopyInLabelIdxs;

        // the wire labels that should be copied to where mCopyInLabelIdxs specifies before a circuit is evaluated
        std::vector<std::vector<block>> mCopyInLabels;

        u64 mPsiResult;
		std::vector<u64> mPSIInputPermutes;
		std::mutex mPsiInputMtx;

#ifdef DUALEX_DEBUG
		std::vector<std::array<block, 2>> DEBUG_theirCommonOutput;
		std::vector<std::vector<std::array<block, 2>>> DEBUG_myDecodedInputs;
#endif

	public:
#ifdef ASYNC_PSI
		AsyncPsiSender mPsiSend;
		AsyncPsiReceiver mPsiRecv;
#else
		PsiSender mPsiSend;
		PsiReceiver mPsiRecv;
#endif

	};

}
