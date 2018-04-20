#pragma once

#include "common/common.h"

#include "block.h"
#include "block_index.h"
#include "transaction.h"
#include "common/threadpool.h"

namespace P2pClouds {

	class Blockchain;

	typedef std::vector<BlockIndex*> BlockChain;

	class Chain : public std::enable_shared_from_this<Chain>
	{
	public:
		Chain();
		virtual ~Chain();

		size_t height();

		bool validBlockTime(time_t timeval);

		void setTip(BlockIndex* pBlockIndex);

		BlockIndex* tip()
		{
			std::lock_guard<std::recursive_mutex> lg(mutex_);

			if (chain_.size() == 0)
				return NULL;

			return chain_[chain_.size() - 1];
		}

		BlockIndex* getBlock(size_t startBlockHeight, size_t blockOffsetHeight);

		time_t getMedianBlockTimePastInChain(size_t range = 11);

	protected:
		BlockChain chain_;
		std::recursive_mutex mutex_;
	};

	typedef std::shared_ptr<Chain> ChainPtr;

	class ChainManager : public std::enable_shared_from_this<ChainManager>
	{
	public:
		// activeChain that do not clear the blocks within this value
		static const int activeChainMinHeight = 288;

		ChainManager(Blockchain* pBlockchain);
		virtual ~ChainManager();

		std::recursive_mutex& mutex() {
			return mutex_;
		}

		ChainPtr& activeChain()
		{
			std::lock_guard<std::recursive_mutex> lg(mutex_);
			return activeChain_;
		}

		size_t activeChainHeight() {
			return activeChain_->height();
		}

		BlockIndex* tip() {
			return activeChain_->tip();
		}

		BlockIndex* getBlock(size_t startBlockHeight, size_t blockOffsetHeight) {
			return activeChain_->getBlock(startBlockHeight, blockOffsetHeight);
		}

		time_t getMedianBlockTimePastInChain(size_t range = 11) {
			return activeChain_->getMedianBlockTimePastInChain(range);
		}

		BlockIndex* acceptBlock(BlockPtr pBlock);
		BlockIndex* addToBlockIndex(BlockPtr pBlock);
		bool activateBestChain(BlockPtr pBlock);
		bool receiveBlock(BlockPtr pBlock, BlockIndex* pBlockIndex);
		bool validBlockHeader(BlockPtr pBlock);
		bool validBlock(BlockPtr pBlock);
		bool validTransaction(Transaction* pTransaction);

		arith_uint256 caculateChainWork(BlockIndex* pBlockIndex);

		BlockMap& mapBlockIndex() {
			return mapBlockIndex_;
		}

		BlockIndex* findBlockIndex(const uint256_t& blockHash)
		{
			std::lock_guard<std::recursive_mutex> lg(mutex_);

			auto iter = mapBlockIndex_.find(blockHash);
			if (iter == mapBlockIndex_.end())
			{
				return NULL;
			}

			return iter->second;
		}

		void clearAllMapBlockIndexs()
		{
			std::lock_guard<std::recursive_mutex> lg(mutex_);

			for (auto& item : mapBlockIndex_)
			{
				delete item.second;
			}

			mapBlockIndex_.clear();
		}

		bool eraseMapBlockIndex(const uint256_t& blockHash)
		{
			std::lock_guard<std::recursive_mutex> lg(mutex_);

			auto iter = mapBlockIndex_.find(blockHash);
			if (iter != mapBlockIndex_.end())
			{
				delete iter->second;
				mapBlockIndex_.erase(iter);
				return true;
			}

			return false;
		}

		bool checkAllBlockIndexs();

		typedef std::map<uint256_t, int32_t> BlockNetNodeIDMap;
		BlockNetNodeIDMap& mapBlockNetNodeID() {
			return mapBlockNetNodeID_;
		}

		int32_t findBlockNetNodeID(const uint256_t& blockHash)
		{
			std::lock_guard<std::recursive_mutex> lg(mutex_);

			auto iter = mapBlockNetNodeID_.find(blockHash);
			if (iter == mapBlockNetNodeID_.end())
			{
				return NULL;
			}

			return iter->second;
		}

		BlockIndex* findMostWorkChain();

	protected:
		Blockchain* pBlockchain_;

		ChainPtr activeChain_;
		std::recursive_mutex mutex_;

		// Block candidate, when a block height is greater than tipBlock, it is placed here as a candidate block when it is not linked to the chain.
		typedef std::set<BlockIndex*, BlockIndex::BlockIndexWorkComparator> SetBlockIndexCandidates;
		SetBlockIndexCandidates blockIndexCandidates;

		// All pairs A->B, where A (or one if its ancestors) misses transactions, but B has transactions.
		std::multimap<BlockIndex*, BlockIndex*> mapUnlinkedBlocks;

		// Including all known block indexes will only increase, not decrease
		BlockMap mapBlockIndex_;

		// Block mapping to NetNodeID
		BlockNetNodeIDMap mapBlockNetNodeID_;

		// Every received block is assigned a unique and increasing identifier, so we know which one to give priority in case of a fork.
		// Blocks loaded from disk are assigned id 0, so start the counter at 1.
		uint32_t BlockSequenceIDCounter_;
	};

	typedef std::shared_ptr<ChainManager> ChainManagerPtr;
}
