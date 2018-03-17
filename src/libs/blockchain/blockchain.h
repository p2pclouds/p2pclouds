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

		uint32_t createNewTransaction(const std::string& sender, const std::string& recipient, uint32_t amount);

		BlockPtr lastBlock();
		BlockPtr getBlock(size_t index, size_t startIndex);

		typedef std::list< BlockPtr > BlockList;
		BlockList& chain() {
			std::lock_guard<std::recursive_mutex> lg(mutex_);
			return chain_;
		}

		uint32_t chainSize() const {
			return chainSize_;
		}

		std::vector< TransactionPtr >& currentTransactions() {
			std::lock_guard<std::recursive_mutex> lg(mutex_);
			return currentTransactions_;
		}

		bool addBlockToChain(BlockPtr pBlock);

        ConsensusPtr pConsensus();
        
		bool start(int numThreads = std::thread::hardware_concurrency());

	protected:
		BlockList chain_;
		uint32_t chainSize_;

        ConsensusPtr pConsensus_;
		std::vector< TransactionPtr > currentTransactions_;

        ThreadPool< ThreadContex >* pThreadPool_;
        std::recursive_mutex mutex_;
	};

}
