#include "transaction.h"
#include "common/byte_buffer.h"

#include <openssl/opensslv.h>
#include <openssl/sha.h>

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

	std::string Transaction::getHash() const
	{
		std::vector<std::uint8_t> hash(SHA256_DIGEST_LENGTH);
		ByteBuffer stream;

		stream << sender_ << recipient_ << amount_;

		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, stream.data(), stream.length());
		SHA256_Final(hash.data(), &sha256);

		return "";
	}
}
