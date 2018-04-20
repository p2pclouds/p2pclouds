#include "chain.h"
#include "consensus.h"
#include "blockchain.h"
#include "merkle.h"
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
		, mapUnlinkedBlocks()
		, mapBlockIndex_()
		, mapBlockNetNodeID_()
		, BlockSequenceIDCounter_(1)
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

	bool ChainManager::validTransaction(Transaction* pTransaction)
	{
		return true;
	}

	bool ChainManager::validBlockHeader(BlockPtr pBlock)
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

	bool ChainManager::validBlock(BlockPtr pBlock)
	{
		if (!validBlockHeader(pBlock))
			return false;

		bool mutated;
		uint256_t hashMerkleRoot = BlockMerkleRoot(*pBlock, &mutated);

		if (hashMerkleRoot != pBlock->pBlockHeader()->hashMerkleRoot)
		{
			LOG_ERROR("hashMerkleRoot mismatch! {}, block: {}", hashMerkleRoot.toString(), 
				pBlock->pBlockHeader()->hashMerkleRoot.toString());

			return false;
		}

		if (mutated)
		{
			LOG_ERROR("duplicate transaction!");
			return false;
		}

		// Size limits
		TRANSACTIONS& blockTransactions = pBlock->transactions();

		ConsensusArgs* pArgs = pBlockchain_->pConsensusArgs();

		uint32_t serializeSize = pBlock->getSerializeSize();
		if (blockTransactions.empty() || blockTransactions.size() >  pArgs->maxBlockWeight || serializeSize >  pArgs->maxBlockWeight)
		{
			LOG_ERROR("size limits failed! blockTransactions={}, serializeSize={}", blockTransactions.size(), serializeSize);
			return false;
		}

		// First transaction must be coinbase, the rest must not be
		if (!blockTransactions[0]->isValueBase())
		{
			LOG_ERROR("first tx is not valueBase! size={}", blockTransactions.size());
			return false;
		}

		for (unsigned int i = 1; i < blockTransactions.size(); i++)
		{
			if (blockTransactions[i]->isValueBase())
			{
				LOG_ERROR("more than one valueBase! size={}", blockTransactions.size());
				return false;
			}
		}

		unsigned int nSigOps = 0;

		// Check transactions
		for (auto& item : blockTransactions)
		{
			if (!validTransaction(item.get()))
			{
				return false;
			}

			//nSigOps += GetLegacySigOpCount(tx);
		}

		if (nSigOps > pArgs->maxBlockSigopsCost)
		{
			LOG_ERROR("out of bounds SigOpCount! nSigOps={}", nSigOps);
			return false;
		}

		return true;
	}

	BlockIndex* ChainManager::acceptBlock(BlockPtr pBlock)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		uint256_t blockHash = pBlock->getHash();

		bool isBlockGenesis = (!pBlockchain_->pConsensusArgs() || blockHash == pBlockchain_->pConsensusArgs()->hashBlockGenesis);

		// Store to disk
		BlockIndex* pBlockIndex = NULL;
		BlockIndex* pFindIndex = findBlockIndex(blockHash);

		if (pFindIndex)
		{
			pBlockIndex = pFindIndex;

			if (!pBlockIndex->isValid())
			{
				LOG_ERROR("accept error block! status={}, block: {}", pBlockIndex->status, pBlock->pBlockHeader()->toString());
				return NULL;
			}
		}

		if (!validBlockHeader(pBlock))
			return NULL;

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

			if (!pBlockIndexPrev->isValid())
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

		if (!isBlockGenesis && !validBlock(pBlock))
			return NULL;

		if (!receiveBlock(pBlock, pBlockIndex))
		{
			delete pBlockIndex;
			return NULL;
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
		std::lock_guard<std::recursive_mutex> lg(mutex_);

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

	bool ChainManager::receiveBlock(BlockPtr pBlock, BlockIndex* pBlockIndex)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		pBlockIndex->numBlockTransactions = pBlock->transactions().size();
		pBlockIndex->numChainTransactions = 0;
		pBlockIndex->status |= BlockIndex::HAVE_DATA;

		// If pindexNew is the genesis block or all parents are BlockIndex::STATUS_HAVE_DATA.
		if (pBlockIndex->pPrev == NULL || pBlockIndex->pPrev->numChainTransactions) 
		{
			std::deque<BlockIndex*> queue;
			queue.push_back(pBlockIndex);

			while (!queue.empty()) 
			{
				BlockIndex *pCurrBlockIndex = queue.front();
				queue.pop_front();

				pCurrBlockIndex->numChainTransactions = (pCurrBlockIndex->pPrev ? pCurrBlockIndex->pPrev->numChainTransactions : 0) + pCurrBlockIndex->numBlockTransactions;
				pCurrBlockIndex->sequenceID = BlockSequenceIDCounter_++;

				// Add to candidate block, chainWork > tip
				if (activeChain_->tip() == NULL || !blockIndexCandidates.value_comp()(pCurrBlockIndex, activeChain_->tip())) {
					blockIndexCandidates.insert(pCurrBlockIndex);
				}

				// Find all other blocks that need to link to this block and add to the candidate
				std::pair<std::multimap<BlockIndex*, BlockIndex*>::iterator, 
					std::multimap<BlockIndex*, BlockIndex*>::iterator> range = mapUnlinkedBlocks.equal_range(pCurrBlockIndex);

				while (range.first != range.second) 
				{
					std::multimap<BlockIndex*, BlockIndex*>::iterator it = range.first;
					queue.push_back(it->second);
					range.first++;
					mapUnlinkedBlocks.erase(it);
				}
			}
		}
		else
		{
			if (pBlockIndex->pPrev && pBlockIndex->pPrev->isValid(BlockIndex::VALID_TREE)) 
			{
				mapUnlinkedBlocks.insert(std::make_pair(pBlockIndex->pPrev, pBlockIndex));
			}
		}

		return true;
	}

	BlockIndex* ChainManager::findMostWorkChain()
	{
		do
		{
			BlockIndex* pBlockIndexNew = NULL;

			// get the best candidate header.
			// It is the largest block of chainWork
			SetBlockIndexCandidates::reverse_iterator it = blockIndexCandidates.rbegin();
			if (it == blockIndexCandidates.rend())
				return NULL;

			pBlockIndexNew = *it;

			// Check whether all blocks on the path between the currently active chain and the candidate are valid.
			// Just going until the active chain is an optimization, as we know all blocks in it are valid already.
			BlockIndex *pBlockIndexTest = pBlockIndexNew;
			bool invalidAncestor = false;


			if (!invalidAncestor)
				return pBlockIndexNew;

		} while (true);

		return NULL;
	}

	bool ChainManager::activateBestChain(BlockPtr pBlock)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		BlockIndex* pBlockIndexNewTip = NULL;
		BlockIndex* pBlockIndexMostWork = NULL;

		do
		{
			pBlockIndexMostWork = findMostWorkChain();

			// Whether we have anything to do at all.
			if (pBlockIndexMostWork == NULL || pBlockIndexMostWork == activeChain_->tip())
				return true;


		} while (pBlockIndexMostWork != activeChain_->tip());

//		return activeChain_->addBlockToChain(pBlock);
		return true;
	}
}
