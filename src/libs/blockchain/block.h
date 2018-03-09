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

		Block(const BlockHeader &header)
			: BlockHeader()
			, index_(0)
			, transactions_()
		{
			*(static_cast<BlockHeader*>(this)) = header;
		}

		virtual ~Block();

		typedef std::vector< TransactionPtr > TRANSACTIONS;

		void transactions(const TRANSACTIONS& vals) {
			transactions_ = vals;
		}

		void addTransactions(const TRANSACTIONS& vals) {
			transactions_.insert(transactions_.end(), vals.begin(), vals.end());
		}

		TRANSACTIONS& transactions() {
			return transactions_;
		}

		void index(uint32_t val) {
			index_ = val;
		}

		uint32_t index() const {
			return index_;
		}

		BlockHeader blockHeader() const
		{
			BlockHeader block;
			block.version = version;
			block.hashPrevBlock = hashPrevBlock;
			block.hashMerkleRoot = hashMerkleRoot;
			block.timestamp = timestamp;
			block.bits = bits;
			block.proof = proof;
			return block;
		}

	protected:
		uint32_t index_;
		TRANSACTIONS transactions_;
	};

	typedef std::shared_ptr<Block> BlockPtr;
}
