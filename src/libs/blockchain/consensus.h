#pragma once

#include "common/common.h"

namespace P2pClouds {

    class Block;
    typedef std::shared_ptr<Block> BlockPtr;

	class BlockIndex;

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

		uint256_t hashBlockGenesis;
	};

	class Consensus : public std::enable_shared_from_this<Consensus>
	{
	public:
		Consensus(Blockchain* pBlockchain, ConsensusArgsPtr args);
		virtual ~Consensus();

        virtual bool build() = 0;
		virtual bool validBlock(BlockPtr pBlock) = 0;
		virtual void createGenesisBlock() = 0;
		virtual BlockPtr createNewBlock(uint32_t bits, uint32_t proof, unsigned int extraProof, BlockIndex* pTipBlockIndex) = 0;

        Blockchain* pBlockchain() const {
            return pBlockchain_;
        }
        
		ConsensusArgsPtr pArgs() {
			return pArgs_;
		}

	protected:
		ConsensusArgsPtr pArgs_;
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
        
		virtual  bool build() override;
		virtual bool validBlock(BlockPtr pBlock) override;
		virtual bool validProofOfWork(const uint256_t& hash, uint32_t bits);
        
		virtual void createGenesisBlock() override;

		virtual BlockPtr createNewBlock(uint32_t bits, uint32_t proof, unsigned int extraProof, BlockIndex* pTipBlockIndex) override;

        uint32_t getNextWorkTarget(BlockPtr pBlock, BlockIndex* pLastBlockIndex);
        uint32_t getWorkTarget(BlockPtr pBlock);
        uint32_t calculateNextWorkTarget(BlockPtr pBlock, BlockIndex* pLastBlockIndex);
        uint64_t calculateSubsidyValue(uint32_t blockHeight);
    };
    
	typedef std::shared_ptr<Consensus> ConsensusPtr;
}

