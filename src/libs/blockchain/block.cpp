#include "block.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {

	uint256_t BlockHeader::getHash() const
	{
		ByteBuffer stream;

		stream << version;
		hashPrevBlock.serialize(stream);
		hashMerkleRoot.serialize(stream);
		stream << timestamp << proof;

		Hash256 hash2561;
		hash2561.update(stream);

		Hash256 hash2562;
		hash2562.update(hash2561.getHash().begin(), uint256::WIDTH);
		return hash2562.getHash();
	}

	Block::Block()
		: BlockHeader()
		, index_(0)
		, transactions_()
	{
	}

	Block::~Block()
	{
	}
}
