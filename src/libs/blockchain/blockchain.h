#pragma once

#include "common/common.h"

#include "block.h"
#include "transaction.h"
#include "common/threadpool.h"

namespace P2pClouds {

    class Consensus;
    typedef std::shared_ptr<Consensus> ConsensusPtr;

	class Blockchain
	{
	public:
		Blockchain();
		virtual ~Blockchain();

		void createGenesisBlock();

		BlockPtr createNewBlock(uint32_t proof, unsigned int extraProof, const uint256_t& hashPrevBlock, bool pushToChain = true);
		uint32_t createNewTransaction(const std::string& sender, const std::string& recipient, uint32_t amount);

		BlockPtr lastBlock();

		std::list< BlockPtr >& chain() {
			return chain_;
		}

		void addBlockToChain(BlockPtr& pBlock)
		{
            std::lock_guard<std::mutex> lg(mutex_);
			chain_.push_back(pBlock);
		}

        ConsensusPtr pConsensus();
        
		bool start(int numThreads = std::thread::hardware_concurrency());

	protected:
		std::list< BlockPtr > chain_;
        ConsensusPtr pConsensus_;
		std::vector< TransactionPtr > currentTransactions_;
        ThreadPool< ThreadContex >* pThreadPool_;
        std::mutex mutex_;
	};

}
