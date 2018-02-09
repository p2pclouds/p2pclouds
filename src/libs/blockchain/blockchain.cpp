#include "blockchain.h"

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
		createNewBlock(100, "1");
	}

	BlockPtr Blockchain::createNewBlock(uint32 proof, const std::string& previousHash)
	{
		BlockPtr pBlock = std::make_shared<Block>();

		pBlock->index((uint32)chain().size() + 1);
		pBlock->timestamp((uint32)(getTimeStamp() & 0xfffffffful));
		pBlock->proof(proof);
		pBlock->transactions(currentTransactions_);
		pBlock->previousHash(previousHash.size() ? previousHash : lastBlock()->getHash());

		currentTransactions_.clear();
		chain_.push_back(pBlock);
		return pBlock;
	}

	uint32 Blockchain::createNewTransaction(const std::string& sender, const std::string& recipient, uint32 amount)
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
}
