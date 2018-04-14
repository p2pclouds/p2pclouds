#pragma once

#include "common/common.h"

#include "chain.h"
#include "common/threadpool.h"

namespace P2pClouds {

    class Consensus;
    typedef std::shared_ptr<Consensus> ConsensusPtr;

	class ConsensusArgs;
	typedef std::shared_ptr<ConsensusArgs> ConsensusArgsPtr;

	class Blockchain
	{
	public:
		Blockchain();
		virtual ~Blockchain();

		TransactionPtr createNewTransaction(const std::string& sender, const std::string& recipient, uint32_t value);

		BlockIndex* processNewBlock(BlockPtr pBlock);

		ChainManagerPtr& chainManager()
		{
			return chainManager_;
		}

		std::vector< TransactionPtr >& currentTransactions() 
		{
			std::lock_guard<std::recursive_mutex> lg(mutex_);
			return currentTransactions_;
		}

        ConsensusPtr pConsensus();
        
		bool start(int numThreads = std::thread::hardware_concurrency());

		std::string userHash() const {
			return userHash_;
		}
	
		uint64_t userGas() const {
			return userGas_;
		}
		
		std::recursive_mutex& mutex() {
			return mutex_;
		}

		BlockMap& mapBlockIndex() {
			return chainManager_->mapBlockIndex();
		}

		ConsensusArgs* pConsensusArgs();

	protected:
		ChainManagerPtr chainManager_;

        ConsensusPtr pConsensus_;
		std::vector< TransactionPtr > currentTransactions_;

        ThreadPool< ThreadContex >* pThreadPool_;
        std::recursive_mutex mutex_;

		std::string userHash_;
		uint64_t userGas_;

	};

}
