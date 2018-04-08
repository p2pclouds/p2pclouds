#pragma once

#include "common/common.h"

namespace P2pClouds {

    class Block;
    typedef std::shared_ptr<Block> BlockPtr;

    class Blockchain;
	class ConsensusArgs;
	typedef std::shared_ptr<ConsensusArgs> ConsensusArgsPtr;

	class ConsensusArgs : public std::enable_shared_from_this<ConsensusArgs>
	{
	public:
		enum ARGS_TYPE
		{
			NORMAL = 0,
			TEST = 1
		};

		static ConsensusArgsPtr create(ARGS_TYPE type);

		// Target difficulty
		arith_uint256 p_difficulty_1_target;
		arith_uint256 b_difficulty_1_target;

		// How many blocks are one cycle, cycle adjustment difficulty
		uint32_t cycleBlockHeight;
		// The theoretical time of a cycle
		uint32_t cycleTimestamp;

		// Interval for declining rewards for bookkeepers
		uint32_t subsidyHalvingInterval;

		// Unit of reward value
		uint64_t valueUnit;
	};

	class Consensus : public std::enable_shared_from_this<Consensus>
	{
	public:
		Consensus(Blockchain* pBlockchain, ConsensusArgsPtr args);
		virtual ~Consensus();

        virtual bool build() = 0;
        virtual bool validBlock(BlockPtr pBlock);
        virtual bool validBlockTime(time_t timeval);

        Blockchain* pBlockchain() const {
            return pBlockchain_;
        }
        
		ConsensusArgsPtr pConsensusArgs() {
			return pConsensusArgs_;
		}

	protected:
		ConsensusArgsPtr pConsensusArgs_;
        Blockchain* pBlockchain_;
	};

    class ConsensusPow : public Consensus
    {
    public:
        static arith_uint256 p_difficulty_1_target;
        static arith_uint256 b_difficulty_1_target;
        static uint32_t cycleBlockHeight;
        static uint32_t cycleTimestamp;
        static uint32_t subsidyHalvingInterval;
        static uint64_t valueUnit;

    public:
        ConsensusPow(Blockchain* pBlockchain, ConsensusArgsPtr args);
        virtual ~ConsensusPow();
        
        bool build() override;
        bool validBlock(BlockPtr pBlock) override;
        bool validBlockTime(time_t timeval) override;
        bool validProofOfWork(const uint256_t& hash, uint32_t proof, uint32_t bits);
        
    	void createGenesisBlock();

		bool addBlockToChain(BlockPtr pBlock);
		BlockPtr createNewBlock(uint32_t bits, uint32_t proof, unsigned int extraProof, BlockPtr pLastBlock, bool pushToChain = true);

        uint32_t getNextWorkTarget(BlockPtr pBlock, BlockPtr pLastBlock);
        uint32_t getWorkTarget(BlockPtr pBlock);
        uint32_t calculateNextWorkTarget(BlockPtr pBlock, BlockPtr pLastBlock);
        uint64_t calculateSubsidyValue(uint32_t blockHeight);
		arith_uint256 caculateChainWork(BlockPtr pBlock);
    };
    
	typedef std::shared_ptr<Consensus> ConsensusPtr;
}

