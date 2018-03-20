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

		uint32_t createNewTransaction(const std::string& sender, const std::string& recipient, uint32_t value);

		BlockPtr lastBlock();
		BlockPtr getBlock(size_t startBlockHeight, size_t blockOffsetHeight);
		BlockPtr getPrevBlock(BlockPtr pBlock);

		time_t getMedianBlockTimePastInChain(size_t range = 11);

		bool addBlockToChain(BlockPtr pBlock);

		typedef std::list< BlockPtr > BlockList;
		BlockList& chain() 
		{
			std::lock_guard<std::recursive_mutex> lg(mutex_);
			return chain_;
		}

		uint32_t chainHeight() const {
			return chainHeight_;
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


	protected:
		BlockList chain_;
		uint32_t chainHeight_;

        ConsensusPtr pConsensus_;
		std::vector< TransactionPtr > currentTransactions_;

        ThreadPool< ThreadContex >* pThreadPool_;
        std::recursive_mutex mutex_;

		std::string userHash_;
		uint64_t userGas_;
	};

}
