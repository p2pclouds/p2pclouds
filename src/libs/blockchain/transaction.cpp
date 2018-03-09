#include "transaction.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {

	Transaction::Transaction()
		: sender_()
		, recipient_()
		, amount_(0)
		, proof_(0)
	{
	}

	Transaction::~Transaction()
	{
	}

	uint256_t Transaction::getHash() const
	{
		ByteBuffer stream;
		stream << sender_ << recipient_ << amount_ << proof_;

		Hash256 hash2561;
		hash2561.update(stream);

		Hash256 hash2562;
		hash2562.update(hash2561.getHash().begin(), uint256::WIDTH);
		return hash2562.getHash();
	}
}
