#pragma once

#include "common/common.h"

#include "transaction.h"

namespace P2pClouds {

	class Block : public std::enable_shared_from_this<Block>
	{
	public:
		Block();
		virtual ~Block();

		void index(uint32_t val) {
			index_ = val;
		}

		uint32_t index() const {
			return index_;
		}

		void timestamp(uint32_t val) {
			timestamp_ = val;
		}

		uint32_t timestamp() const {
			return timestamp_;
		}

		void transactions(const std::list< TransactionPtr >& vals) {
			transactions_ = vals;
		}

		std::list< TransactionPtr >& transactions() {
			return transactions_;
		}

		void proof(uint32_t val) {
			proof_ = val;
		}

		uint32_t proof() const {
			return proof_;
		}

		void previousHash(const uint256_t& val) {
			previousHash_ = val;
		}

		uint256_t previousHash() const {
			return previousHash_;
		}

		uint256_t getHash() const;

	protected:
		uint32_t index_;
		uint32_t timestamp_;
		std::list< TransactionPtr > transactions_;
		uint32_t proof_;
		uint256_t previousHash_;
	};

	typedef std::shared_ptr<Block> BlockPtr;
}
