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

		void amount(uint32 val) {
			amount_ = val;
		}

		uint32 amount() const {
			return amount_;
		}

	protected:
		std::string sender_;
		std::string recipient_;
		uint32 amount_;
	};

	typedef std::shared_ptr<Transaction> TransactionPtr;
}
