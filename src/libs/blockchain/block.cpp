#include "block.h"

namespace P2pClouds {

	Block::Block()
		: index_(0)
		, timestamp_(0)
		, transactions_()
		, proof_(0)
		, previousHash_()
	{
	}

	Block::~Block()
	{
	}

	
}
