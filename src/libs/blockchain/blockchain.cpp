#include "blockchain.h"
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
		createNewBlock(100, uint256S("1"));
	}

	BlockPtr Blockchain::createNewBlock(uint32_t proof, const uint256_t& hashPrevBlock)
	{
		BlockPtr pBlock = std::make_shared<Block>();

		pBlock->index((uint32_t)chain().size() + 1);
		pBlock->timestamp = (uint32_t)(getTimeStamp() & 0xfffffffful);
		pBlock->proof = proof;
		pBlock->transactions(currentTransactions_);
		pBlock->hashPrevBlock = hashPrevBlock.size() ? hashPrevBlock : lastBlock()->getHash();

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

	bool Blockchain::proofOfWork(BlockPtr pBlock)
	{
		pBlock->proof = 0;

		for (; pBlock->proof < pow(2, 32); ++pBlock->proof)
		{
			if (validProof(pBlock->getHash(), pBlock->proof, pBlock->bits))
				return true;
		}

		return false;
	}

	bool Blockchain::validProof(const uint256_t& hash, uint32_t proof, uint32_t bits)
	{
		bool isNegative;
		bool isOverflow;

		arith_uint256 target;
		target.setCompact(bits, &isNegative, &isOverflow);

		// Check range
		if (isNegative || target == 0 || isOverflow || target > p_difficulty_1_target)
			return false;

		arith_uint256 hashResult(hash.toString());

		if (hashResult > target)
			return false;

		return true;
	}

	bool Blockchain::mine()
	{

		while(true)
		{
			BlockPtr pLastblock = lastBlock();
			BlockPtr pNewBlock = createNewBlock(0, pLastblock->getHash());

			arith_uint256 target;
			target.setCompact(pNewBlock->bits);

			float difficulty = (b_difficulty_1_target / target).getdouble();

			LOG_DEBUG("Starting search...");

			time_t start_timestamp = getTimeStamp();

			if (proofOfWork(pNewBlock))
			{
				float elapsedTime = float(getTimeStamp() - start_timestamp) / 1000.f;
				float hashPower = pNewBlock->proof / elapsedTime;

				// Generate a globally unique address for this node
				createNewTransaction("0", pNewBlock->getHash().toString(), 1);

				LOG_DEBUG("Success with proof: {}", pNewBlock->proof);
				LOG_DEBUG("Hash: {}", pNewBlock->getHash().toString());
				LOG_DEBUG("Elapsed Time: {} seconds", elapsedTime);
				LOG_DEBUG("Hashing Power: {} hashes per second", hashPower);
				LOG_DEBUG("Difficulty: {} ({} bits)", difficulty, pNewBlock->bits);
				LOG_DEBUG("");
			}
			else
			{
				LOG_ERROR("Failed after {} (maxProof) tries)", pNewBlock->proof);
				LOG_DEBUG("");
			}
		}

		return true;
	}

}
