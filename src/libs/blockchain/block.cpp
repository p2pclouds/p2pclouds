#include "block.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {

	uint256_t BlockHeader::getHash() const
	{
		ByteBuffer stream;
        
        stream.clear(false);
        serialize(stream);

        uint256_t hash2561;
        SHA256(stream.data(), stream.length(), (unsigned char*)&hash2561);

        uint256_t hash2562;
        SHA256(hash2561.begin(), uint256::WIDTH, (unsigned char*)&hash2562);
		return hash2562;
	}

    void BlockHeader::serialize(ByteBuffer& stream) const
    {
        stream << version;
        hashPrevBlock.serialize(stream);
        hashMerkleRoot.serialize(stream);
        stream << timestamp << bits << proof;
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
