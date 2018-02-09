#include "block.h"
#include "common/byte_buffer.h"

#include <openssl/opensslv.h>
#include <openssl/sha.h>

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

	std::string Block::getHash() const
	{
		std::vector<std::uint8_t> hash(SHA256_DIGEST_LENGTH);
		ByteBuffer stream;

		stream << index_ << timestamp_ << proof_ << previousHash_;

		for (auto& item : transactions_)
			stream << item->getHash();

		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, stream.data(), stream.length());
		SHA256_Final(hash.data(), &sha256);

		return "";
	}
}
