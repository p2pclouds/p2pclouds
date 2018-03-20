#include "blockchain.h"
#include "merkle.h"
#include "consensus.h"
#include "common/hash.h"

namespace P2pClouds {

	Blockchain::Blockchain()
		: chain_()
		, chainHeight_(0)
        , pConsensus_()
		, currentTransactions_()
        , pThreadPool_(NULL)
        , mutex_()
		, userHash_()
		, userGas_(0)
	{
        pConsensus_ = std::shared_ptr<Consensus>(new ConsensusPow(this));
	}

	Blockchain::~Blockchain()
	{
        SAFE_RELEASE(pThreadPool_);
	}

	bool Blockchain::addBlockToChain(BlockPtr pBlock)
	{
        std::lock_guard<std::recursive_mutex> lg(mutex_);

		if(pBlock->height() != (chainHeight_ + 1))
			return false;

		if(pConsensus_ && !pConsensus_->validBlock(pBlock))
			return false;

		chain_.push_back(pBlock);
		++chainHeight_;

		currentTransactions_.erase(currentTransactions_.begin(), 
			currentTransactions_.begin() + (pBlock->transactions().size() - 1));

		return true;
	}

	time_t Blockchain::getMedianBlockTimePastInChain(size_t range)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);
		std::vector<time_t> blockTimes;

		BlockList::reverse_iterator rit = chain_.rbegin();
		for (; rit != chain_.rend(); ++rit)
		{
        	blockTimes.push_back((*rit)->pBlockHeader()->getTimeval());

			if(blockTimes.size() == range)
				break;
		}

		std::sort(blockTimes.begin(), blockTimes.end(), std::less<time_t>());

		return blockTimes[blockTimes.size() / 2];
	}

	uint32_t Blockchain::createNewTransaction(const std::string& sender, const std::string& recipient, uint32_t value)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		TransactionPtr pTransaction = std::make_shared<Transaction>();
		pTransaction->value(value);
		pTransaction->recipient(recipient);
		pTransaction->sender(sender);

        currentTransactions_.push_back(pTransaction);

		return chainHeight() > 0 ? (lastBlock()->height() + 1) : 0;
	}

	BlockPtr Blockchain::lastBlock()
	{
        std::lock_guard<std::recursive_mutex> lg(mutex_);
		return chain_.back();
	}

	BlockPtr Blockchain::getBlock(size_t startBlockHeight, size_t blockOffsetHeight)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		BlockList::reverse_iterator rit = chain_.rbegin();
		for (; rit != chain_.rend(); ++rit)
		{
			if(startBlockHeight == 0 || startBlockHeight == (*rit)->height())
			{
				startBlockHeight = 0;
				if(blockOffsetHeight == 0 || --blockOffsetHeight == 0)
        			return (*rit);
			}
		}

		LOG_ERROR("not found block! startBlockHeight={}, blockOffsetHeight={}, chainHeight={}", 
			startBlockHeight, blockOffsetHeight, chainHeight());

		return BlockPtr(NULL);
	}

	BlockPtr Blockchain::getPrevBlock(BlockPtr pBlock)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		BlockList::reverse_iterator rit = chain_.rbegin();
		for (; rit != chain_.rend(); ++rit)
		{
			if(pBlock->pBlockHeader()->hashPrevBlock == (*rit)->getHash())
			{
        		return (*rit);
			}
		}

		LOG_ERROR("not found prevBlock! height={}, hashPrevBlock={}", pBlock->height(), 
			pBlock->pBlockHeader()->hashPrevBlock.toString());

		return BlockPtr(NULL);
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

