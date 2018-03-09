#include "block.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {

	uint256_t BlockHeader::getHash() const
	{
		static ByteBuffer stream;
        
        stream.clear(false);
		stream << version;
		hashPrevBlock.serialize(stream);
		hashMerkleRoot.serialize(stream);
		stream << timestamp << proof;

        uint256_t hash2561;
        SHA256(stream.data(), stream.length(), (unsigned char*)&hash2561);

        uint256_t hash2562;
        SHA256(hash2561.begin(), uint256::WIDTH, (unsigned char*)&hash2562);
		return hash2562;
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
