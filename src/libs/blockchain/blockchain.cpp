#include "blockchain.h"
#include "merkle.h"
#include "consensus.h"
#include "common/hash.h"

namespace P2pClouds {

	Blockchain::Blockchain()
		: chain_()
        , pConsensus_()
		, currentTransactions_()
        , pThreadPool_(NULL)
        , mutex_()
	{
        pConsensus_ = std::shared_ptr<Consensus>(new ConsensusPow(this));
		createGenesisBlock();
	}

	Blockchain::~Blockchain()
	{
        SAFE_RELEASE(pThreadPool_);
	}

	void Blockchain::createGenesisBlock()
	{
		createNewBlock(0, 0, uint256S("1"));
	}

	BlockPtr Blockchain::createNewBlock(uint32_t proof, unsigned int extraProof, const uint256_t& hashPrevBlock, bool pushToChain)
	{
		BlockPtr pBlock = std::make_shared<Block>(new BlockHeaderPoW());
        BlockHeaderPoW* pBlockHeaderPoW = (BlockHeaderPoW*)pBlock->pBlockHeader();
        
		pBlock->index((uint32_t)chain().size() + 1);
		pBlockHeaderPoW->timestamp = (uint32_t)(getTimeStamp() & 0xfffffffful);
		pBlockHeaderPoW->proof = proof;
		pBlockHeaderPoW->hashPrevBlock = hashPrevBlock.size() ? hashPrevBlock : lastBlock()->getHash();

		// coin base
		TransactionPtr pBaseTransaction = std::make_shared<Transaction>();
		pBaseTransaction->proof(extraProof);
		pBaseTransaction->amount(0);
		pBaseTransaction->recipient("0");
		pBaseTransaction->sender("0");
		pBlock->transactions().push_back(pBaseTransaction);

		// packing Transactions
		pBlock->addTransactions(currentTransactions_);
        pBlockHeaderPoW->hashMerkleRoot = BlockMerkleRoot(*pBlock);
		
		if (pushToChain)
			addBlockToChain(pBlock);

        std::lock_guard<std::mutex> lg(mutex_);
        currentTransactions_.clear();
		return pBlock;
	}

	uint32_t Blockchain::createNewTransaction(const std::string& sender, const std::string& recipient, uint32_t amount)
	{
		TransactionPtr pTransaction = std::make_shared<Transaction>();
		pTransaction->amount(amount);
		pTransaction->recipient(recipient);
		pTransaction->sender(sender);

        size_t chainSizeS = 0;
        {
            std::lock_guard<std::mutex> lg(mutex_);
            currentTransactions_.push_back(pTransaction);
            chainSizeS = chain_.size();
        }

		return chainSizeS > 0 ? (lastBlock()->index() + 1) : 0;
	}

	BlockPtr Blockchain::lastBlock()
	{
        std::lock_guard<std::mutex> lg(mutex_);
		return chain_.back();
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

