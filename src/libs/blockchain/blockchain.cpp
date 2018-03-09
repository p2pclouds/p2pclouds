#include "blockchain.h"
#include "merkle.h"
#include "common/hash.h"

namespace P2pClouds {

	arith_uint256 Blockchain::p_difficulty_1_target("0x00000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
	arith_uint256 Blockchain::b_difficulty_1_target("0x00000000FFFF0000000000000000000000000000000000000000000000000000");

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
		createNewBlock(0, 0, uint256S("1"));
	}

	BlockPtr Blockchain::createNewBlock(uint32_t proof, unsigned int extraProof, const uint256_t& hashPrevBlock, bool pushToChain)
	{
		BlockPtr pBlock = std::make_shared<Block>();

		pBlock->index((uint32_t)chain().size() + 1);
		pBlock->timestamp = (uint32_t)(getTimeStamp() & 0xfffffffful);
		pBlock->proof = proof;
		pBlock->hashPrevBlock = hashPrevBlock.size() ? hashPrevBlock : lastBlock()->getHash();

		// coin base
		TransactionPtr pBaseTransaction = std::make_shared<Transaction>();
		pBaseTransaction->proof(extraProof);
		pBaseTransaction->amount(0);
		pBaseTransaction->recipient("0");
		pBaseTransaction->sender("0");
		pBlock->transactions().push_back(pBaseTransaction);

		// packing Transactions
		pBlock->addTransactions(currentTransactions_);

		currentTransactions_.clear();
		pBlock->hashMerkleRoot = BlockMerkleRoot(*pBlock);

		if (pushToChain)
			addBlockToChain(pBlock);

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

	bool Blockchain::validProofOfWork(const uint256_t& hash, uint32_t proof, uint32_t bits)
	{
		bool isNegative;
		bool isOverflow;

		arith_uint256 target;
		target.setCompact(bits, &isNegative, &isOverflow);

		// Check range
		if (isNegative || target == 0 || isOverflow || target > p_difficulty_1_target)
			return false;

        arith_uint256 hashResult;
        uintToArith256(hashResult, hash);

		if (hashResult > target)
			return false;

		return true;
	}

	bool Blockchain::mine()
	{
		//LOG_DEBUG("Starting search...");

		const int innerLoopCount = 0x1000000;
		uint64_t maxTries = pow(2, 32) - 1;
		unsigned int extraProof = 0;

		time_t start_timestamp = 0;
		BlockPtr pLastblock = lastBlock();
		BlockPtr pFoundBlock;
		float difficulty = 1.f;

		while (maxTries > 0)
		{
			BlockPtr pNewBlock = createNewBlock(0, extraProof++, pLastblock->getHash(), false);

			arith_uint256 target;
			target.setCompact(pNewBlock->bits);

			difficulty = (float)(b_difficulty_1_target / target).getdouble();
            start_timestamp = getTimeStamp();
            
			while (maxTries > 0 && pNewBlock->proof < innerLoopCount && !validProofOfWork(pNewBlock->getHash(), pNewBlock->proof, pNewBlock->bits))
			{
				--maxTries;
				++pNewBlock->proof;
			}

			if (pNewBlock->proof >= innerLoopCount)
            {
				//LOG_ERROR("Failed after {} (maxProof) tries)", pNewBlock->proof);
				//LOG_DEBUG("");
				continue;
			}

			if (maxTries == 0) 
			{
				//LOG_ERROR("Failed after {} (maxTries) tries)", 1000000 - maxTries);
				//LOG_DEBUG("");
				break;
			}

			pFoundBlock = pNewBlock;
			break;
		}

		if (!pFoundBlock)
			return false;

		float elapsedTime = float(getTimeStamp() - start_timestamp) / 1000.f;
		float hashPower = pFoundBlock->proof / elapsedTime;

		addBlockToChain(pFoundBlock);

		LOG_DEBUG("Success with proof: {}", pFoundBlock->proof);
		LOG_DEBUG("Hash: {}", pFoundBlock->getHash().toString());
		LOG_DEBUG("Elapsed Time: {} seconds", elapsedTime);
		LOG_DEBUG("Hashing Power: {} hashes per second", hashPower);
		LOG_DEBUG("Difficulty: {} ({} bits)", difficulty, pFoundBlock->bits);
		LOG_DEBUG("");

		return true;
	}

}

