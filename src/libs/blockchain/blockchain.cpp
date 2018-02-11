#include "blockchain.h"
#include "common/hash.h"

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

	uint32_t Blockchain::proofOfWork(const uint256_t& payloadHash, uint32_t bits)
	{
		uint32_t proof = 0;

		while (!validProof(payloadHash, proof, bits))
		{
			++proof;
		}

		return proof;
	}

	bool Blockchain::validProof(const uint256_t& payloadHash, uint32_t proof, uint32_t bits)
	{
		Hash256 hash256;

		hash256.update(&proof, sizeof(proof));
		hash256.update(payloadHash.begin(), payloadHash.size());

		std::string guess = hash256.getHash().toString();

		for(uint32_t i=0; i<bits; ++i)
		{
			if (guess[i] != '0')
				return false;
		}

		printf("proofOfWork---%s, proof=%d\n", guess.c_str(), proof);
		return true;
	}

	bool Blockchain::mine()
	{
		BlockPtr lastblock = lastBlock();

		uint32_t proof = proofOfWork(lastblock->getHash(), lastblock->bits());

		// Generate a globally unique address for this node
		std::string nodeIdentifier = "myHashaddr";
		createNewTransaction("0", nodeIdentifier, 1);

		createNewBlock(proof, lastblock->getHash());

		return true;
	}

}
