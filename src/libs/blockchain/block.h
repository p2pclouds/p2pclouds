#pragma once

#include "common/common.h"

#include "transaction.h"

namespace P2pClouds {
    class ByteBuffer;
    
    class BlockHeader
    {
    public:
		int32_t version;
		uint256 hashPrevBlock;
		uint256 hashMerkleRoot;
		uint32_t timeval;

        BlockHeader()
			: version(P2PCLOUDS_VERSION)
			, hashPrevBlock()
			, hashMerkleRoot()
			, timeval(0)
		{
        }
        
        virtual ~BlockHeader()
        {
        }
        
		int64_t getTimeval() const { return (int64_t)timeval; }

        virtual uint256_t getHash() const = 0;
		
        virtual void serialize(ByteBuffer& stream) const;
    };
    
    class BlockHeaderPoW : public BlockHeader
	{
	public:
		uint32_t bits;
		uint32_t proof;

		BlockHeaderPoW()
			: BlockHeader()
			, bits(0)
			, proof(0)

		{
		}

        virtual ~BlockHeaderPoW()
        {
        }
        
		uint256_t getHash() const override;
        void serialize(ByteBuffer& stream) const override;
	};

	class Block : public std::enable_shared_from_this<Block>
	{
	public:
		Block();

		Block(BlockHeader* pBlockHeader)
			: height_(0)
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

		void height(uint32_t val) {
			height_ = val;
		}

		uint32_t height() const {
			return height_;
		}

		BlockHeader* pBlockHeader()
		{
			return pBlockHeader_;
		}

        virtual uint256_t getHash() const {
            return pBlockHeader_->getHash();
        }
        
	protected:
		uint32_t height_;
		TRANSACTIONS transactions_;
        BlockHeader* pBlockHeader_;
	};

	typedef std::shared_ptr<Block> BlockPtr;
}
