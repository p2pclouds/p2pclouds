#include "transaction.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {

	Transaction::Transaction()
		: sender_()
		, recipient_()
		, amount_(0)
		, magic_(0)
	{
	}

	Transaction::~Transaction()
	{
	}

	uint256_t Transaction::getHash() const
	{
		ByteBuffer stream;
		stream << sender_ << recipient_ << amount_ << magic_;

        uint256_t hash2561;
        SHA256(stream.data(), stream.length(), (unsigned char*)&hash2561);
        
        uint256_t hash2562;
        SHA256(hash2561.begin(), uint256::WIDTH, (unsigned char*)&hash2562);
        return hash2562;
	}
}
