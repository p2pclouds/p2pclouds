#pragma once

#include "common/common.h"

namespace P2pClouds {

    class Block;
    typedef std::shared_ptr<Block> BlockPtr;

    class Blockchain;
    
	class Consensus : public std::enable_shared_from_this<Consensus>
	{
	public:
		Consensus(Blockchain* pBlockchain);
		virtual ~Consensus();

        virtual bool build() = 0;
        virtual bool validBlock(BlockPtr pBlock);
        virtual bool validBlockTime(time_t timeval);

        Blockchain* pBlockchain() const {
            return pBlockchain_;
        }
        
	protected:
        Blockchain* pBlockchain_;
	};

    class ConsensusPow : public Consensus
    {
    public:
        static arith_uint256 p_difficulty_1_target;
        static arith_uint256 b_difficulty_1_target;
        
    public:
        ConsensusPow(Blockchain* pBlockchain);
        virtual ~ConsensusPow();
        
        bool build() override;
        bool validBlock(BlockPtr pBlock) override;
        bool validBlockTime(time_t timeval) override;
        bool validProofOfWork(const uint256_t& hash, uint32_t proof, uint32_t bits);
        
    	void createGenesisBlock();

		BlockPtr createNewBlock(uint32_t bits, uint32_t proof, unsigned int extraProof, BlockPtr pLastBlock, bool pushToChain = true);

        uint32_t getNextWorkTarget(BlockPtr pBlock, BlockPtr pLastBlock);
        uint32_t calculateNextWorkTarget(BlockPtr pBlock, BlockPtr pLastBlock);

    protected:
    };
    
	typedef std::shared_ptr<Consensus> ConsensusPtr;
}

