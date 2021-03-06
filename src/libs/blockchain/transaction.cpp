#include "transaction.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {

	Transaction::Transaction()
		: sender_()
		, recipient_()
		, value_(0)
		, magic_(0)
	{
	}

	Transaction::~Transaction()
	{
	}

	uint256_t Transaction::getHash() const
	{
		ByteBuffer stream;
		stream << sender_ << recipient_ << value_ << magic_;

        uint256_t hash2561;
        SHA256(stream.data(), stream.length(), (unsigned char*)&hash2561);
        
        uint256_t hash2562;
        SHA256(hash2561.begin(), uint256_t::WIDTH, (unsigned char*)&hash2562);
        return hash2562;
	}

	uint32_t Transaction::getSerializeSize()
	{
		uint32_t size = 0;
		return 0;
	}
}
