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

	uint256_t Block::getHash() const
	{
		uint256_t hash;

		ByteBuffer stream;

		stream << index_ << timestamp_ << proof_;
		
		previousHash_.serialize(stream);

		for (auto& item : transactions_)
			stream << item->getHash();

		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, stream.data(), stream.length());
		SHA256_Final((unsigned char*)&hash, &sha256);

		return hash;
	}
}
