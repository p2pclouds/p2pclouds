#pragma once

#include "common/common.h"

#include "transaction.h"

namespace P2pClouds {
    class ByteBuffer;
    
    class BlockHeader
    {
    public:
        BlockHeader() {
        }
        
        virtual ~BlockHeader()
        {
        }
        
        virtual uint256_t getHash() const = 0;
        virtual void serialize(ByteBuffer& stream) const = 0;
    };
    
    class BlockHeaderPoW : public BlockHeader
	{
	public:
		int32_t version;
		uint256 hashPrevBlock;
		uint256 hashMerkleRoot;
		uint32_t timeval;
		uint32_t bits;
		uint32_t proof;

		BlockHeaderPoW()
			: BlockHeader()
            , version(P2PCLOUDS_VERSION)
			, hashPrevBlock()
			, hashMerkleRoot()
			, timeval(0)
			, bits(0x1d00ffff)
			, proof(0)

		{
		}

        virtual ~BlockHeaderPoW()
        {
        }
        
		int64_t getTimeval() const { return (int64_t)timeval; }
		uint256_t getHash() const override;
        void serialize(ByteBuffer& stream) const override;
	};

	class Block : public std::enable_shared_from_this<Block>
	{
	public:
		Block();

		Block(BlockHeader* pBlockHeader)
			: index_(0)
			, transactions_()
            , pBlockHeader_(pBlockHeader)
		{
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

		BlockHeader* pBlockHeader()
		{
			return pBlockHeader_;
		}

        virtual uint256_t getHash() const {
            return pBlockHeader_->getHash();
        }
        
	protected:
		uint32_t index_;
		TRANSACTIONS transactions_;
        BlockHeader* pBlockHeader_;
	};

	typedef std::shared_ptr<Block> BlockPtr;
}
