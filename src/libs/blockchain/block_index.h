#pragma once

#include "block.h"
#include "common/common.h"

namespace P2pClouds {

	class BlockIndex : public std::enable_shared_from_this<BlockIndex>
	{
	public:
		enum STATUS 
		{
			// Unused.
			VALID_UNKNOWN = 0,

			// Parsed, version ok, hash satisfies claimed PoW, 1 <= vtx count <= max, timestamp not in future
			VALID_HEADER = 1,

			// All parent headers found, difficulty matches, timestamp >= median previous, 
			// Implies all parents are also at least TREE.
			VALID_TREE = 2,

			/**
			* Only first tx is coinbase, scripts valid, transactions valid, no duplicate txids,
			* sigops, size, merkle root. Implies all parents are at least TREE but not necessarily TRANSACTIONS. When all
			* parent blocks also have TRANSACTIONS, BlockIndex::numChainTransactions will be set.
			*/
			VALID_TRANSACTIONS = 3,

			// Outputs do not overspend inputs, no double spends, coinbase output ok, no immature coinbase spends, BIP30.
			// Implies all parents are also at least CHAIN.
			VALID_CHAIN = 4,

			// Scripts & signatures ok. Implies all parents are also at least SCRIPTS.
			VALID_SCRIPTS = 5,

			// All validity bits.
			VALID_MASK = VALID_HEADER | VALID_TREE | VALID_TRANSACTIONS | VALID_CHAIN | VALID_SCRIPTS,

			// full block available in blk*.dat
			HAVE_DATA = 8,

			// undo data available in rev*.dat
			HAVE_UNDO = 16, 

			HAVE_MASK = HAVE_DATA | HAVE_UNDO,

			// stage after last reached validness failed
			FAILED_VALID = 32, 

			// descends from failed block
			FAILED_CHILD = 64, 

			FAILED_MASK = FAILED_VALID | FAILED_CHILD,
		};

	public:
		BlockIndex(BlockPtr pBlock);
		virtual ~BlockIndex();

		struct BlockIndexWorkComparator
		{
			bool operator()(BlockIndex *pa, BlockIndex *pb) const 
			{
				if (pa->chainWork > pb->chainWork) 
					return false;

				if (pa->chainWork < pb->chainWork) 
					return true;

				if (pa->sequenceID < pb->sequenceID) 
					return false;

				if (pa->sequenceID > pb->sequenceID) 
					return true;

				if (pa < pb) 
					return false;

				if (pa > pb) 
					return true;

				return false;
			}
		};

		struct BlockHasher
		{
			size_t operator()(const uint256_t& hash) const { return hash.GetCheapHash(); }
		};

		std::string toString();

		//! Check whether this block index entry is valid up to the passed validity level.
		bool isValid(STATUS upToLevel/* = VALID_TRANSACTIONS*/) const
		{
			if ((status & BlockIndex::FAILED_MASK) > 0)
				return false;

			return (status & VALID_MASK) >= (uint32_t)upToLevel;
		}

		bool isValid() const
		{
			return (status & BlockIndex::FAILED_MASK) == 0;
		}

		int64_t getTimeval() const { return (int64_t)timeval; }

	public:
		uint32_t height;

		// (memory only)
		arith_uint256 chainWork;

		uint32_t status;

		// pointer to the hash of the block, if any. Memory is owned by this BlockIndex
		const uint256* phashBlock;

		// pointer to the index of the predecessor of this block
		BlockIndex* pPrev;

		// block header
		int32_t version;
		uint256_t hashMerkleRoot;
		uint32_t timeval;
		uint32_t bits;
		uint32_t proof;

		// (memory only) Sequential id assigned to distinguish order in which blocks are received.
		uint32_t sequenceID;

		// Number of transactions in this block.
		uint32_t numBlockTransactions;

		// (memory only) Number of transactions in the chain up to and including this block.
		uint32_t numChainTransactions;
	};

	typedef std::shared_ptr<BlockIndex> BlockIndexPtr;
	typedef std::unordered_map<uint256_t, BlockIndex*, BlockIndex::BlockHasher> BlockMap;

}


