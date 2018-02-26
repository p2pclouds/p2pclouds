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

		BlockPtr createNewBlock(uint32_t proof, const uint256_t& previousHash);
		uint32_t createNewTransaction(const std::string& sender, const std::string& recipient, uint32_t amount);

		BlockPtr lastBlock();

		std::string hashBlock(BlockPtr block);

		std::list< BlockPtr >& chain() {
			return chain_;
		}

		bool proofOfWork(const uint256_t& payloadHash, uint32_t bits, uint32_t& outProof, uint256_t& outHash);
		bool validProof(const uint256_t& payloadHash, uint32_t proof, uint32_t bits, uint256_t& outHash);

		bool mine();

	protected:
		std::list< BlockPtr > chain_;
		std::list< TransactionPtr > currentTransactions_;
	};

}
