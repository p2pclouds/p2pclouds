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

	BlockIndex* Chain::findFork(BlockIndex* pBlockIndex)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		if (pBlockIndex->height > height())
			pBlockIndex = pBlockIndex->getAncestor(height());

		/*
		                  * <--- fork pos
						  |
		   |1|2|3|4|5|6|7|8|8|9|10|10|
		*/
		while (pBlockIndex && !contains(pBlockIndex))
			pBlockIndex = pBlockIndex->pPrev;

		return pBlockIndex;
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
		bool isBlockGenesis = pBlockchain_->isGenesisHash(blockHash);

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

		if (!checkAllBlockIndexs() || !activateBestChain(pBlock))
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

			// build skiplist pointer
			pIndexNew->buildSkip();
		}

		pIndexNew->chainWork = (pIndexNew->pPrev ? pIndexNew->pPrev->chainWork : 0) + caculateChainWork(pIndexNew);

		return pIndexNew;
	}

	bool ChainManager::checkAllBlockIndexs()
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		if (activeChain_->height() < 0)
		{
			assert(mapBlockIndex_.size() == 1);
			return pBlockchain_->isGenesisHash(*mapBlockIndex_.begin()->second->phashBlock);
		}

		// Build forward-pointing map of the entire block tree.
		std::multimap<BlockIndex*, BlockIndex*> forward;
		for (BlockMap::iterator it = mapBlockIndex_.begin(); it != mapBlockIndex_.end(); it++) 
		{
			forward.insert(std::make_pair(it->second->pPrev, it->second));
		}

		assert(forward.size() == mapBlockIndex_.size());

		std::pair<std::multimap<BlockIndex*, BlockIndex*>::iterator, std::multimap<BlockIndex*, BlockIndex*>::iterator> rangeGenesis = forward.equal_range(NULL);
		BlockIndex *pIndex = rangeGenesis.first->second;
		
		// There is only one index entry with parent NULL. should be GenesisBlock
		rangeGenesis.first++;
		assert(rangeGenesis.first == rangeGenesis.second); 
		return true;

		size_t numNodes = 0;
		int height = 0;

		BlockIndex* pindexFirstInvalid = NULL; // Oldest ancestor of pindex which is invalid.
		BlockIndex* pindexFirstMissing = NULL; // Oldest ancestor of pindex which does not have BLOCK_HAVE_DATA.
		BlockIndex* pindexFirstNeverProcessed = NULL; // Oldest ancestor of pindex for which nTx == 0.
		BlockIndex* pindexFirstNotTreeValid = NULL; // Oldest ancestor of pindex which does not have BLOCK_VALID_TREE (regardless of being valid or not).
		BlockIndex* pindexFirstNotTransactionsValid = NULL; // Oldest ancestor of pindex which does not have BLOCK_VALID_TRANSACTIONS (regardless of being valid or not).
		BlockIndex* pindexFirstNotChainValid = NULL; // Oldest ancestor of pindex which does not have BLOCK_VALID_CHAIN (regardless of being valid or not).
		BlockIndex* pindexFirstNotScriptsValid = NULL; // Oldest ancestor of pindex which does not have BLOCK_VALID_SCRIPTS (regardless of being valid or not).

		while (pIndex)
		{
			++numNodes;

			if (pindexFirstInvalid == NULL && pIndex->status & BlockIndex::FAILED_VALID) pindexFirstInvalid = pIndex;
			if (pindexFirstMissing == NULL && !(pIndex->status & BlockIndex::HAVE_DATA)) pindexFirstMissing = pIndex;
			if (pindexFirstNeverProcessed == NULL && pIndex->numBlockTransactions == 0) pindexFirstNeverProcessed = pIndex;
			if (pIndex->pPrev != NULL && pindexFirstNotTreeValid == NULL && (pIndex->status & BlockIndex::VALID_MASK) < BlockIndex::VALID_TREE) pindexFirstNotTreeValid = pIndex;
			if (pIndex->pPrev != NULL && pindexFirstNotTransactionsValid == NULL && (pIndex->status & BlockIndex::VALID_MASK) < BlockIndex::VALID_TRANSACTIONS) pindexFirstNotTransactionsValid = pIndex;
			if (pIndex->pPrev != NULL && pindexFirstNotChainValid == NULL && (pIndex->status & BlockIndex::VALID_MASK) < BlockIndex::VALID_CHAIN) pindexFirstNotChainValid = pIndex;
			if (pIndex->pPrev != NULL && pindexFirstNotScriptsValid == NULL && (pIndex->status & BlockIndex::VALID_MASK) < BlockIndex::VALID_SCRIPTS) pindexFirstNotScriptsValid = pIndex;

			if (pIndex->pPrev == NULL) 
			{
				// Genesis block checks.
				assert(pBlockchain_->isGenesisHash(*pIndex->phashBlock)); // Genesis block's hash must match.
				assert(pIndex == activeChain_->getGenesisBlockIndex()); // The current active chain's genesis block must be this block.
			}

			// sequenceId can't be set for blocks that aren't linked
			if (pIndex->numChainTransactions == 0)
				assert(pIndex->sequenceID == 0);

			// height must be consistent.
			assert(pIndex->height == height); 
			assert(height < 2 || (pIndex->pSkip && (pIndex->pSkip->height < height))); // The pskip pointer must point back for all but the first 2 blocks.
			assert(pindexFirstNotTreeValid == NULL); // All mapBlockIndex entries must at least be TREE valid


			if (!BlockIndex::BlockIndexWorkComparator()(pIndex, activeChain_->tip()) && pindexFirstNeverProcessed == NULL) {
				if (pindexFirstInvalid == NULL) 
				{
					// If this block sorts at least as good as the current tip and
					// is valid and we have all data for its parents, it must be in
					// setBlockIndexCandidates.  chainActive.Tip() must also be there
					// even if some data has been pruned.
					if (pindexFirstMissing == NULL || pIndex == activeChain_->tip()) 
					{
						assert(blockIndexCandidates.count(pIndex));
					}

					// If some parent is missing, then it could be that this block was in
					// setBlockIndexCandidates but had to be removed because of the missing data.
					// In this case it must be in mapBlocksUnlinked -- see test below.
				}
			}
			else 
			{ 
				// If this block sorts worse than the current tip or some ancestor's block has never been seen, it cannot be in setBlockIndexCandidates.
				assert(blockIndexCandidates.count(pIndex) == 0);
			}

			// Check whether this block is in mapBlocksUnlinked.
			std::pair<std::multimap<BlockIndex*, BlockIndex*>::iterator, std::multimap<BlockIndex*, BlockIndex*>::iterator> rangeUnlinked = mapUnlinkedBlocks.equal_range(pIndex->pPrev);
			bool foundInUnlinked = false;
			while (rangeUnlinked.first != rangeUnlinked.second) 
			{
				assert(rangeUnlinked.first->first == pIndex->pPrev);
				if (rangeUnlinked.first->second == pIndex) {
					foundInUnlinked = true;
					break;
				}
				rangeUnlinked.first++;
			}

			if (pIndex->pPrev && (pIndex->status & BlockIndex::HAVE_DATA) && pindexFirstNeverProcessed != NULL && pindexFirstInvalid == NULL) {
				// If this block has block data available, some parent was never received, and has no invalid parents, it must be in mapBlocksUnlinked.
				assert(foundInUnlinked);
			}
			if (!(pIndex->status & BlockIndex::HAVE_DATA)) assert(!foundInUnlinked); // Can't be in mapBlocksUnlinked if we don't HAVE_DATA
			if (pindexFirstMissing == NULL) assert(!foundInUnlinked); // We aren't missing data for any parent -- cannot be in mapBlocksUnlinked.

			if (pIndex->pPrev && (pIndex->status & BlockIndex::HAVE_DATA) && pindexFirstNeverProcessed == NULL && pindexFirstMissing != NULL) 
			{
				// We HAVE_DATA for this block, have received data for all parents at some point, but we're currently missing data for some parent.
			//	assert(fHavePruned); // We must have pruned.
									 // This block may have entered mapBlocksUnlinked if:
									 //  - it has a descendant that at some point had more work than the
									 //    tip, and
									 //  - we tried switching to that descendant but were missing
									 //    data for some intermediate block between chainActive and the
									 //    tip.
									 // So if this block is itself better than chainActive.Tip() and it wasn't in
									 // setBlockIndexCandidates, then it must be in mapBlocksUnlinked.
				if (!BlockIndex::BlockIndexWorkComparator()(pIndex, activeChain_->tip()) && blockIndexCandidates.count(pIndex) == 0) {
					if (pindexFirstInvalid == NULL) {
						assert(foundInUnlinked);
					}
				}
			}

			// assert(pindex->GetBlockHash() == pindex->GetBlockHeader().GetHash()); // Perhaps too slow
			// End: actual consistency checks.

			// Try descending into the first subnode.
			std::pair<std::multimap<BlockIndex*, BlockIndex*>::iterator, std::multimap<BlockIndex*, BlockIndex*>::iterator> range = forward.equal_range(pIndex);
			if (range.first != range.second) 
			{
				// A subnode was found.
				pIndex = range.first->second;
				height++;
				continue;
			}

			while (pIndex) {
				// We are going to either move to a parent or a sibling of pindex.
				// If pindex was the first with a certain property, unset the corresponding variable.
				if (pIndex == pindexFirstInvalid) pindexFirstInvalid = NULL;
				if (pIndex == pindexFirstMissing) pindexFirstMissing = NULL;
				if (pIndex == pindexFirstNeverProcessed) pindexFirstNeverProcessed = NULL;
				if (pIndex == pindexFirstNotTreeValid) pindexFirstNotTreeValid = NULL;
				if (pIndex == pindexFirstNotTransactionsValid) pindexFirstNotTransactionsValid = NULL;
				if (pIndex == pindexFirstNotChainValid) pindexFirstNotChainValid = NULL;
				if (pIndex == pindexFirstNotScriptsValid) pindexFirstNotScriptsValid = NULL;

				// Find our parent.
				BlockIndex* pindexPar = pIndex->pPrev;

				// Find which child we just visited.
				std::pair<std::multimap<BlockIndex*, BlockIndex*>::iterator, std::multimap<BlockIndex*, BlockIndex*>::iterator> rangePar = forward.equal_range(pindexPar);
				while (rangePar.first->second != pIndex) {
					assert(rangePar.first != rangePar.second); // Our parent must have at least the node we're coming from as child.
					rangePar.first++;
				}

				// Proceed to the next one.
				rangePar.first++;

				if (rangePar.first != rangePar.second) 
				{
					// Move to the sibling.
					pIndex = rangePar.first->second;
					break;
				}
				else 
				{
					// Move up further.
					pIndex = pindexPar;
					height--;
					continue;
				}
			}
		}

		// Check that we actually traversed the entire map.
		assert(numNodes == forward.size());

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

	void ChainManager::pruneBlockIndexCandidates()
	{
		// Note that we can't delete the current block itself, as we may need to return to it later in case a
		// reorganization to a better block fails.
		auto it = blockIndexCandidates.begin();

		while (it != blockIndexCandidates.end() && blockIndexCandidates.value_comp()(*it, activeChain_->tip()))
		{
			blockIndexCandidates.erase(it++);
		}

		assert(!blockIndexCandidates.empty());
	}

	BlockIndex* ChainManager::findMostWorkBlockIndex()
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
			BlockIndex *pBlockIndexBestInvalid = NULL;
			bool invalidAncestor = false;

			// Test, if the current block is not found on the activity chain, it will always look up to a previous node
			while (pBlockIndexTest && !activeChain_->contains(pBlockIndexTest))
			{
				assert(pBlockIndexTest->numChainTransactions || pBlockIndexTest->height == 0);

				// Pruned nodes may have entries in setBlockIndexCandidates for
				// which block files have been deleted.  Remove those as candidates
				// for the most work chain if we come across them; we can't switch
				// to a chain unless we have all the non-active-chain parent blocks.
				bool failedChain = !pBlockIndexTest->isValid();
				bool missingData = !(pBlockIndexTest->status & BlockIndex::HAVE_DATA);

				if (failedChain || missingData)
				{
					// Candidate chain is not usable (either invalid or missing data)
					if (failedChain && (pBlockIndexBestInvalid == NULL || pBlockIndexNew->chainWork > pBlockIndexBestInvalid->chainWork))
						pBlockIndexBestInvalid = pBlockIndexNew;

					BlockIndex* pBlockIndexFailed = pBlockIndexNew;

					// Remove the entire chain from the set.
					while (pBlockIndexTest != pBlockIndexFailed)
					{
						if (failedChain)
						{
							pBlockIndexFailed->status |= BlockIndex::FAILED_CHILD;
						}
						else if (missingData)
						{
							// If we're missing data, then add back to mapBlocksUnlinked,
							// so that if the block arrives in the future we can try adding
							// to setBlockIndexCandidates again.
							mapUnlinkedBlocks.insert(std::make_pair(pBlockIndexFailed->pPrev, pBlockIndexFailed));
						}

						blockIndexCandidates.erase(pBlockIndexFailed);
						pBlockIndexFailed = pBlockIndexFailed->pPrev;
					}

					blockIndexCandidates.erase(pBlockIndexTest);
					invalidAncestor = true;
					break;
				}

				pBlockIndexTest = pBlockIndexTest->pPrev;
			}

			if (!invalidAncestor)
				return pBlockIndexNew;

		} while (true);

		return NULL;
	}

	bool ChainManager::activateBestChainStep(BlockIndex* pBlockIndexMostWork, BlockPtr pBlock)
	{
		bool invalidFound = false;
		const BlockIndex *pIndexOldTip = activeChain_->tip();
		const BlockIndex *pIndexFork = activeChain_->findFork(pBlockIndexMostWork);

		// Disconnect active blocks which are no longer in the best chain.
		while (activeChain_->tip() && activeChain_->tip() != pIndexFork) 
		{
			if (!disconnectTip())
				return false;
		}

		// Build list of new blocks to connect.
		std::vector<BlockIndex*> blockIndexToConnects;
		bool isContinue = true;
		int height = pIndexFork ? pIndexFork->height : -1;

		while (isContinue && height != pBlockIndexMostWork->height)
		{
			int targetHeight = std::min(height + 32, (int)pBlockIndexMostWork->height);

			blockIndexToConnects.clear();
			blockIndexToConnects.reserve(targetHeight - height);

			// Get blockindex->pPrev at targetHeight
			BlockIndex *pIndexIter = pBlockIndexMostWork->getAncestor(targetHeight);

			// All BlockIndex from pBlockIndexMostWork to fork
			while (pIndexIter && pIndexIter->height != height)
			{
				blockIndexToConnects.push_back(pIndexIter);
				pIndexIter = pIndexIter->pPrev;
			}

			height = targetHeight;

			for (auto& pIndexConnect : blockIndexToConnects)
			{
				if (!connectTip(pIndexConnect, pIndexConnect == pBlockIndexMostWork ? pBlock : NULL))
				{
					invalidFound = true;
					isContinue = false;
				}
				else 
				{
					pruneBlockIndexCandidates();

					if (!pIndexOldTip || activeChain_->tip()->chainWork > pIndexOldTip->chainWork)
					{
						// We're in a better position than we were. Return temporarily to release the lock.
						isContinue = false;
						break;
					}
				}
			}
		}

		return true;
	}

	bool ChainManager::activateBestChain(BlockPtr pBlock)
	{
		std::lock_guard<std::recursive_mutex> lg(mutex_);

		BlockIndex* pBlockIndexNewTip = NULL;
		BlockIndex* pBlockIndexMostWork = NULL;

		do
		{
			pBlockIndexMostWork = findMostWorkBlockIndex();

			// If pBlockIndexMostWork is not found, it is usually because there is no candidate block, or it is not a valid block
			// Or, If pBlockIndexMostWork is the head of the activity chain,
			// We don't need to do anything
			if (pBlockIndexMostWork == NULL || pBlockIndexMostWork == activeChain_->tip())
				return true;

			if (!activateBestChainStep(pBlockIndexMostWork, pBlock && pBlock->getHash() == *pBlockIndexMostWork->phashBlock ? pBlock : NULL))
				return false;

			pBlockIndexNewTip = activeChain_->tip();

			// load for disk or download
			bool initialBlock = isInitialBlock();
			if (!initialBlock)
			{
				// send block to networknodes
			}

		} while (pBlockIndexMostWork != activeChain_->tip());

		if (!checkAllBlockIndexs())
			return false;

		return true;
	}

	bool ChainManager::isInitialBlock()
	{
		return false;
	}

	bool ChainManager::connectTip(BlockIndex* pBlockIndexNew, BlockPtr pBlock)
	{
		updateTip(pBlockIndexNew);
		return true;
	}

	bool ChainManager::disconnectTip()
	{
		BlockIndex *pBlockIndexDelete = activeChain_->tip();
		updateTip(pBlockIndexDelete->pPrev);
		return true;
	}

	void ChainManager::updateTip(BlockIndex* pBlockIndexNew)
	{
		activeChain_->setTip(pBlockIndexNew);
	}
}
