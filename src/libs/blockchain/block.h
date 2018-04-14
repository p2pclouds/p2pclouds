#pragma once

#include "common/common.h"

#include "transaction.h"

namespace P2pClouds {
    class ByteBuffer;
    
    class BlockHeader
    {
    public:
		int32_t version;
		uint256_t hashPrevBlock;
		uint256_t hashMerkleRoot;
		uint32_t timeval;
		uint32_t bits;
		uint32_t proof;

        BlockHeader()
			: version(P2PCLOUDS_VERSION)
			, hashPrevBlock()
			, hashMerkleRoot()
			, timeval(0)
			, bits(0)
			, proof(0)
		{
        }
        
        virtual ~BlockHeader()
        {
        }

		uint256_t getHash() const;
		void serialize(ByteBuffer& stream) const;

		std::string toString();
    };

	class Block : public std::enable_shared_from_this<Block>
	{
	public:
		Block();

		Block(BlockHeader* pBlockHeader)
			: transactions_()
            , pBlockHeader_(pBlockHeader)
		{
		}

		virtual ~Block();

		void transactions(const TRANSACTIONS& vals) {
			transactions_ = vals;
		}

		void addTransactions(const TRANSACTIONS& vals) {
			transactions_.insert(transactions_.end(), vals.begin(), vals.end());
		}

		TRANSACTIONS& transactions() {
			return transactions_;
		}

		BlockHeader* pBlockHeader()
		{
			return pBlockHeader_;
		}

        virtual uint256_t getHash() const {
            return pBlockHeader_->getHash();
        }

	protected:
		TRANSACTIONS transactions_;
        BlockHeader* pBlockHeader_;
	};

	typedef std::shared_ptr<Block> BlockPtr;
}
