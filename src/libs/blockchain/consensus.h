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
        bool validProofOfWork(const uint256_t& hash, uint32_t proof, uint32_t bits);
        
    	void createGenesisBlock();

		BlockPtr createNewBlock(uint32_t proof, unsigned int extraProof, const uint256_t& hashPrevBlock, bool pushToChain = true);

    protected:
    };
    
	typedef std::shared_ptr<Consensus> ConsensusPtr;
}

