#include "transaction.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {

	Transaction::Transaction()
		: sender_()
		, recipient_()
		, amount_(0)
	{
	}

	Transaction::~Transaction()
	{
	}

	uint256_t Transaction::getHash() const
	{
		ByteBuffer stream;
		stream << sender_ << recipient_ << amount_;

		Hash256 hash256;
		hash256.update(stream);

		return hash256.getHash();
	}
}
