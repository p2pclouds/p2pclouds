#pragma once

#include "block.h"
#include "common/common.h"

namespace P2pClouds {

	class BlockIndex : public std::enable_shared_from_this<BlockIndex>
	{
	public:
		enum STATUS {
			STATUS_NORMAL = 0,
			STATUS_ERROR = 1,
			STATUS_HAVE_DATA = 2,
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

		bool valid() const
		{
			return (status & BlockIndex::STATUS_ERROR) == 0;
		}

		int64_t getTimeval() const { return (int64_t)timeval; }

	public:
		uint32_t height;
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
	};

	typedef std::shared_ptr<BlockIndex> BlockIndexPtr;
	typedef std::unordered_map<uint256_t, BlockIndex*, BlockIndex::BlockHasher> BlockMap;

}


