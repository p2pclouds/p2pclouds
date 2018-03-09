#pragma once

#include "common/common.h"

#include "transaction.h"

namespace P2pClouds {

	class BlockHeader
	{
	public:
		int32_t version;
		uint256 hashPrevBlock;
		uint256 hashMerkleRoot;
		uint32_t timestamp;
		uint32_t bits;
		uint32_t proof;

		BlockHeader()
			: version(P2PCLOUDS_VERSION)
			, hashPrevBlock()
			, hashMerkleRoot()
			, timestamp(0)
			, bits(0x1d00ffff)
			, proof(0)

		{
		}

		uint256_t getHash() const;
	};

	class Block : public BlockHeader, public std::enable_shared_from_this<Block>
	{
	public:
		Block();
		virtual ~Block();

		void transactions(const std::list< TransactionPtr >& vals) {
			transactions_ = vals;
		}

		std::list< TransactionPtr >& transactions() {
			return transactions_;
		}

		void index(uint32_t val) {
			index_ = val;
		}

		uint32_t index() const {
			return index_;
		}

	protected:
		uint32_t index_;
		std::list< TransactionPtr > transactions_;
	};

	typedef std::shared_ptr<Block> BlockPtr;
}
