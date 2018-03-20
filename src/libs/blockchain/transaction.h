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

		void value(uint32_t val) {
			value_ = val;
		}

		uint32_t value() const {
			return value_;
		}

		void magic(uint32_t val) {
			magic_ = val;
		}

		uint32_t magic() const {
			return magic_;
		}

		uint256_t getHash() const;

	protected:
		std::string sender_;
		std::string recipient_;
		uint32_t value_;
		uint32_t magic_;
	};

	typedef std::shared_ptr<Transaction> TransactionPtr;
}
