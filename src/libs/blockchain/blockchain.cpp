#include "blockchain.h"
#include "common/hash.h"
#include <openssl/bn.h>

namespace P2pClouds {

	Blockchain::Blockchain()
		: chain_()
		, currentTransactions_()
	{
		createGenesisBlock();
	}

	Blockchain::~Blockchain()
	{
	}

	void Blockchain::createGenesisBlock()
	{
		createNewBlock(100, uint256S("1"));
	}

	BlockPtr Blockchain::createNewBlock(uint32_t proof, const uint256_t& previousHash)
	{
		BlockPtr pBlock = std::make_shared<Block>();

		pBlock->index((uint32_t)chain().size() + 1);
		pBlock->timestamp((uint32_t)(getTimeStamp() & 0xfffffffful));
		pBlock->proof(proof);
		pBlock->transactions(currentTransactions_);
		pBlock->previousHash(previousHash.size() ? previousHash : lastBlock()->getHash());

		currentTransactions_.clear();
		chain_.push_back(pBlock);
		return pBlock;
	}

	uint32_t Blockchain::createNewTransaction(const std::string& sender, const std::string& recipient, uint32_t amount)
	{
		TransactionPtr pTransaction = std::make_shared<Transaction>();
		pTransaction->amount(amount);
		pTransaction->recipient(recipient);
		pTransaction->sender(sender);

		currentTransactions_.push_back(pTransaction);
		return chain_.size() > 0 ? (lastBlock()->index() + 1) : 0;
	}

	BlockPtr Blockchain::lastBlock() 
	{
		return chain_.back();
	}

	bool Blockchain::proofOfWork(const uint256_t& payloadHash, uint32_t bits, uint32_t& outProof, uint256_t& outHash)
	{
		uint32_t proof = 0;

		for (; proof < pow(2, 32); ++proof)
		{
			if (validProof(payloadHash, proof, bits, outHash))
			{
				outProof = proof;
				return true;
			}
		}

		outProof = proof;
		return false;
	}

	bool Blockchain::validProof(const uint256_t& payloadHash, uint32_t proof, uint32_t bits, uint256_t& outHash)
	{
		Hash256 hash256_1;

		hash256_1.update(payloadHash.begin(), payloadHash.size());
		hash256_1.update(&proof, sizeof(proof));

		Hash256 hash256_2;
		hash256_2.update(hash256_1.getHash().begin(), uint256::WIDTH);

		std::string sHashResult = hash256_2.getHash().toString();
		BIGNUM* hashResult = BN_new();

		if(!BN_hex2bn(&hashResult, sHashResult.c_str()))
		{
			LOG_CRITICAL("BN_hex2bn error!");
			return false;
		}

		BN_CTX *ctx = BN_CTX_new();
		if (!ctx)
		{
			LOG_CRITICAL("BN_CTX_new error!");
			return false;
		}

		BIGNUM* target = BN_new();

		if (!BN_set_word(target, 2))
		{
			LOG_CRITICAL("BN_set_word error!");
			return false;
		}

		BIGNUM* difficultyBits = BN_new();
		if (!BN_set_word(difficultyBits, 256 - bits))
		{
			LOG_CRITICAL("BN_set_word error!");
			return false;
		}

		if (!BN_exp(target, target, difficultyBits, ctx))
		{
			LOG_CRITICAL("BN_exp error!");
			return false;
		}

		bool found = BN_cmp(hashResult, target) < 0;

		if(found)
		{
			outHash = uint256S(sHashResult);
		}

		BN_free(target);
		BN_free(difficultyBits);
		BN_free(hashResult);
		BN_CTX_free(ctx);
		return found;
	}

	bool Blockchain::mine()
	{
		for (int difficultyBits = 0; difficultyBits < 32; ++difficultyBits)
		{
			int difficulty = pow(2, difficultyBits);
			BlockPtr lastblock = lastBlock();

			uint32_t proof = 0;
			uint256_t hashResult;

			LOG_DEBUG("Starting search...");

			time_t start_timestamp = getTimeStamp();

			if (proofOfWork(lastblock->getHash(), difficultyBits/*lastblock->bits()*/, proof, hashResult))
			{
				float elapsedTime = float(getTimeStamp() - start_timestamp) / 1000.f;
				float hashPower = proof / elapsedTime;

				// Generate a globally unique address for this node
				createNewTransaction("0", hashResult.toString(), 1);

				createNewBlock(proof, lastblock->getHash());

				LOG_DEBUG("Success with proof: {}", proof);
				LOG_DEBUG("Hash: {}", hashResult.toString());
				LOG_DEBUG("Elapsed Time: {} seconds", elapsedTime);
				LOG_DEBUG("Hashing Power: {} hashes per second", hashPower);
				LOG_DEBUG("Difficulty: {} ({} bits)", difficulty, difficultyBits);
				LOG_DEBUG("");
			}
			else
			{
				LOG_ERROR("Failed after {} (maxProof) tries)", proof);
				LOG_DEBUG("");
			}
		}

		return true;
	}

}
