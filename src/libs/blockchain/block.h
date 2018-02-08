#pragma once

#include "common/common.h"

#include "transaction.h"

namespace P2pClouds {

	class Block : public std::enable_shared_from_this<Block>
	{
	public:
		Block();
		virtual ~Block();

		void index(uint32 val) {
			index_ = val;
		}

		uint32 index() const {
			return index_;
		}

		void timestamp(uint32 val) {
			timestamp_ = val;
		}

		uint32 timestamp() const {
			return timestamp_;
		}

		void transactions(const std::list< TransactionPtr >& vals) {
			transactions_ = vals;
		}

		std::list< TransactionPtr >& transactions() {
			return transactions_;
		}

		void proof(uint32 val) {
			proof_ = val;
		}

		uint32 proof() const {
			return proof_;
		}

		void previousHash(std::string val) {
			previousHash_ = val;
		}

		std::string previousHash() const {
			return previousHash_;
		}

	protected:
		uint32 index_;
		uint32 timestamp_;
		std::list< TransactionPtr > transactions_;
		uint32 proof_;
		std::string previousHash_;
	};

	typedef std::shared_ptr<Block> BlockPtr;
}
