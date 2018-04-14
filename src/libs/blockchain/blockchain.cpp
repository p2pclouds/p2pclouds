#include "blockchain.h"
#include "merkle.h"
#include "consensus.h"
#include "common/hash.h"

namespace P2pClouds {

	Blockchain::Blockchain()
		: chainManager_(NULL)
        , pConsensus_()
		, currentTransactions_()
        , pThreadPool_(NULL)
        , mutex_()
		, userHash_()
		, userGas_(0)
	{
		chainManager_ = std::make_shared<ChainManager>(this);
        pConsensus_ = std::shared_ptr<Consensus>(new ConsensusPow(this, ConsensusArgs::create(ConsensusArgs::NORMAL)));
	}

	Blockchain::~Blockchain()
	{
        SAFE_RELEASE(pThreadPool_);
	}

	ConsensusArgs* Blockchain::pConsensusArgs()
	{
		return pConsensus_ ? pConsensus_->pArgs().get() : NULL;
	}

	BlockIndex* Blockchain::processNewBlock(BlockPtr pBlock)
	{
        std::lock_guard<std::recursive_mutex> lg(mutex_);

		BlockIndex* pBlockIndex = chainManager_->acceptBlock(pBlock);
		if (!pBlockIndex)
			return NULL;

		TRANSACTIONS& transactions = pBlock->transactions();
		if (currentTransactions_.size() > 0 && transactions.size() > 1/* coinbase*/ && 
			currentTransactions_[transactions.size() - 2/* -coinbase*/].get() ==
			transactions[transactions.size() - 1].get())
		{
			currentTransactions_.erase(currentTransactions_.begin(),
				currentTransactions_.begin() + (pBlock->transactions().size() - 1));
		}

		return pBlockIndex;
	}

	TransactionPtr Blockchain::createNewTransaction(const std::string& sender, const std::string& recipient, uint32_t value)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		TransactionPtr pTransaction = std::make_shared<Transaction>();
		pTransaction->value(value);
		pTransaction->recipient(recipient);
		pTransaction->sender(sender);

        currentTransactions_.push_back(pTransaction);

		return pTransaction;
	}

    ConsensusPtr Blockchain::pConsensus()
    {
        return pConsensus_;
    }
    
    bool Blockchain::start(int numThreads)
    {
        if(numThreads == 0)
            numThreads = std::thread::hardware_concurrency();

        SAFE_RELEASE(pThreadPool_);
        pThreadPool_ = new ThreadPool<ThreadContex>(numThreads);
        
        LOG_DEBUG("Starting Blockchain(numThreads={})", numThreads);
        
        for(int i=0; i<numThreads; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            
            Blockchain* pBlockchain = this;
            pThreadPool_->enqueue([pBlockchain](ThreadContex& context)
            {
                while(true)
                    pBlockchain->pConsensus()->build();
                
                return true;
            });
        }
        
        return true;
    }
}

