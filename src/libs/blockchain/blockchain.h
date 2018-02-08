#pragma once

#include "common/common.h"

#include "block.h"
#include "transaction.h"

namespace P2pClouds {

	class Blockchain
	{
	public:
		Blockchain();
		virtual ~Blockchain();

		void createGenesisBlock();

		BlockPtr createNewBlock(uint32 proof, const std::string& previousHash);
		uint32 createNewTransaction(const std::string& sender, const std::string& recipient, uint32 amount);

		BlockPtr lastBlock();

		std::string hashBlock(BlockPtr block);

		std::list< BlockPtr >& chain() {
			return chain_;
		}

	protected:
		std::list< BlockPtr > chain_;
		std::list< TransactionPtr > currentTransactions_;
	};

}
