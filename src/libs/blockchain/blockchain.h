#pragma once

#include "common/common.h"

#include "block.h"
#include "transaction.h"

namespace P2pClouds {

	class Blockchain
	{
	public:
		static arith_uint256 p_difficulty_1_target;
		static arith_uint256 b_difficulty_1_target;

		Blockchain();
		virtual ~Blockchain();

		void createGenesisBlock();

		BlockPtr createNewBlock(uint32_t proof, const uint256_t& hashPrevBlock);
		uint32_t createNewTransaction(const std::string& sender, const std::string& recipient, uint32_t amount);

		BlockPtr lastBlock();

		std::string hashBlock(BlockPtr pBlock);

		std::list< BlockPtr >& chain() {
			return chain_;
		}

		bool proofOfWork(BlockPtr pBlock);
		bool validProof(const uint256_t& hash, uint32_t proof, uint32_t bits);

		bool mine();

	protected:
		std::list< BlockPtr > chain_;
		std::list< TransactionPtr > currentTransactions_;
	};

}
