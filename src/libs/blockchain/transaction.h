#pragma once

#include "common/common.h"

namespace P2pClouds {

	class Transaction : public std::enable_shared_from_this<Transaction>
	{
	public:
		Transaction();
		virtual ~Transaction();

		void sender(std::string val) {
			sender_ = val;
		}

		std::string sender() const {
			return sender_;
		}

		void recipient(std::string val) {
			recipient_ = val;
		}

		std::string recipient() const {
			return recipient_;
		}

		void amount(uint32_t val) {
			amount_ = val;
		}

		uint32_t amount() const {
			return amount_;
		}

		uint256_t getHash() const;

	protected:
		std::string sender_;
		std::string recipient_;
		uint32_t amount_;
	};

	typedef std::shared_ptr<Transaction> TransactionPtr;
}
