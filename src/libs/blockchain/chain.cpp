#include "chain.h"
#include "consensus.h"
#include "blockchain.h"
#include "log/log.h"

namespace P2pClouds {

	Chain::Chain()
		: chain_()
		, mutex_()
	{
	}

	Chain::~Chain()
	{
	}

	size_t Chain::height()
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);
		return chain_.size();
	}

	void Chain::setTip(BlockIndex* pBlockIndex)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		if (pBlockIndex == NULL) 
		{
			chain_.clear();
			return;
		}

		chain_.resize(pBlockIndex->height + 1);

		while (pBlockIndex && chain_[pBlockIndex->height] != pBlockIndex) {
			chain_[pBlockIndex->height] = pBlockIndex;
			pBlockIndex = pBlockIndex->pPrev;
		}
	}

	time_t Chain::getMedianBlockTimePastInChain(size_t range)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		if (chain_.size() == 0)
			return 0;

		std::vector<time_t> blockTimes;

		BlockChain::reverse_iterator rit = chain_.rbegin();
		for (; rit != chain_.rend(); ++rit)
		{
			blockTimes.push_back((*rit)->getTimeval());

			if (blockTimes.size() == range)
				break;
		}

		std::sort(blockTimes.begin(), blockTimes.end(), std::less<time_t>());

		return blockTimes[blockTimes.size() / 2];
	}

	BlockIndex* Chain::getBlock(size_t startBlockHeight, size_t blockOffsetHeight)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		BlockChain::reverse_iterator rit = chain_.rbegin();
		for (; rit != chain_.rend(); ++rit)
		{
			if (startBlockHeight == 0 || startBlockHeight == (*rit)->height)
			{
				startBlockHeight = 0;
				if (blockOffsetHeight == 0 || --blockOffsetHeight == 0)
					return (*rit);
			}
		}

		LOG_ERROR("not found block! startBlockHeight={}, blockOffsetHeight={}, chainHeight={}",
			startBlockHeight, blockOffsetHeight, chain_.size());

		return NULL;
	}

	bool Chain::validBlockTime(time_t timeval)
	{
		if (timeval > getAdjustedTime() + 2 * 60 * 60)
		{
			LOG_ERROR("Illegal timeval({}), not conforming to adjustedTime({})!", timeval, getAdjustedTime());
			return false;
		}

		time_t medianTime = getMedianBlockTimePastInChain();
		if (timeval < medianTime)
		{
			LOG_ERROR("Illegal timeval({}), not conforming to medianTime({})!", timeval, medianTime);
			return false;
		}

		return true;
	}

	ChainManager::ChainManager(Blockchain* pBlockchain)
		: pBlockchain_(pBlockchain)
		, activeChain_(new Chain())
		, mutex_()
		, blockIndexCandidates()
		, mapBlockIndex_()
		, mapBlockNetNodeID_()
	{
	}

	ChainManager::~ChainManager()
	{
		clearAllMapBlockIndexs();
	}

	arith_uint256 ChainManager::caculateChainWork(BlockIndex* pBlockIndex)
	{
		bool isNegative;
		bool isOverflow;

		arith_uint256 target;
		target.setCompact(pBlockIndex->bits, &isNegative, &isOverflow);

		// Check range
		if (isNegative || target == 0 || isOverflow)
			return 0;

		// We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
		// as it's too large for an arith_uint256. However, as 2**256 is at least as large
		// as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
		// or ~bnTarget / (bnTarget+1) + 1.
		return (~target / (target + 1)) + 1;
	}

	bool ChainManager::validBlock(BlockPtr pBlock)
	{
		ConsensusPtr pConsensus = pBlockchain_->pConsensus();

		if (pConsensus)
		{
			if (!pConsensus->validBlock(pBlock) ||
				!activeChain_->validBlockTime(pBlock->pBlockHeader()->timeval))
				return false;
		}

		return true;
	}

	BlockIndex* ChainManager::acceptBlock(BlockPtr pBlock)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		uint256_t blockHash = pBlock->getHash();
		bool isBlockGenesis = (!pBlockchain_->pConsensusArgs() || blockHash == pBlockchain_->pConsensusArgs()->hashBlockGenesis);

		if (!validBlock(pBlock))
			return NULL;

		// Store to disk
		BlockIndex* pBlockIndex = NULL;
		BlockIndex* pFindIndex = findBlockIndex(blockHash);

		if (pFindIndex)
		{
			pBlockIndex = pFindIndex;

			if (!pBlockIndex->valid())
			{
				LOG_ERROR("accept error block! status={}, block: {}", pBlockIndex->status, pBlock->pBlockHeader()->toString());
				return NULL;
			}
		}

		// Get prev block index
		BlockIndex* pBlockIndexPrev = NULL;
		if (!isBlockGenesis)
		{
			pBlockIndexPrev = findBlockIndex(pBlock->pBlockHeader()->hashPrevBlock);

			if (!pBlockIndexPrev)
			{
				LOG_ERROR("not found prev block! block: {}", pBlock->pBlockHeader()->toString());
				return false;
			}

			if (!pBlockIndexPrev->valid())
			{
				LOG_ERROR("prev block invalid! block index: {}", pBlockIndexPrev->toString());
				return false;
			}
		}

		if (pBlockIndex == NULL)
			pBlockIndex = addToBlockIndex(pBlock);

		BlockIndex* pTipBlockIndex = activeChain_->tip();

		//bool haveData = pBlockIndex->status & BlockIndex::STATUS_HAVE_DATA;
		//bool hasMoreWork = (pTipBlockIndex ? pBlockIndex->chainWork > pTipBlockIndex->chainWork : true);
		//bool tooFarAhead = (pBlockIndex->height > int(activeChainHeight() + activeChainMinHeight));

		if (!isBlockGenesis)
		{
			if (!receiveBlock(pBlock))
			{
				delete pBlockIndex;
				return NULL;
			}
		}
		else
		{
			activeChain_->setTip(NULL);
			activeChain_->setTip(pBlockIndex);
			return pBlockIndex;
		}

		if (!activateBestChain(pBlock) || !checkAllBlockIndexs())
		{
			delete pBlockIndex;
			return NULL;
		}

		return pBlockIndex;
	}

	BlockIndex* ChainManager::addToBlockIndex(BlockPtr pBlock)
	{
		BlockMap& mapBlockIndex = pBlockchain_->mapBlockIndex();

		// Construct new block index object
		BlockIndex* pIndexNew = new BlockIndex(pBlock);
		assert(pIndexNew);

		pIndexNew->sequenceID = 0;

		auto iter = mapBlockIndex.insert(std::make_pair(pBlock->getHash(), pIndexNew)).first;
		pIndexNew->phashBlock = &((*iter).first);

		auto iterPrev = mapBlockIndex.find(pBlock->pBlockHeader()->hashPrevBlock);
		if (iterPrev != mapBlockIndex.end())
		{
			pIndexNew->pPrev = (*iterPrev).second;
			pIndexNew->height = pIndexNew->pPrev->height + 1;

			//pIndexNew->buildSkip();
		}

		pIndexNew->chainWork = (pIndexNew->pPrev ? pIndexNew->pPrev->chainWork : 0) + caculateChainWork(pIndexNew);

		return pIndexNew;
	}

	bool ChainManager::checkAllBlockIndexs()
	{
		return true;
	}

	bool ChainManager::receiveBlock(BlockPtr pBlock)
	{
		return true;
	}

	bool ChainManager::activateBestChain(BlockPtr pBlock)
	{
//		return activeChain_->addBlockToChain(pBlock);
		return true;
	}
}
