#include "block.h"
#include "common/byte_buffer.h"
#include "common/hash256.h"

namespace P2pClouds {

	Block::Block()
		: index_(0)
		, timestamp_(0)
		, transactions_()
		, previousHash_()
		, proof_(0)
		, bits_(4)
	{
	}

	Block::~Block()
	{
	}

	uint256_t Block::getHash() const
	{
		ByteBuffer stream;

		stream << index_ << timestamp_ << proof_;
		previousHash_.serialize(stream);

		for (auto& item : transactions_)
			stream << item->getHash();

		Hash256 hash256;
		hash256.update(stream);

		return hash256.getHash();
	}
}
