#include "block.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {

	uint256_t BlockHeaderPoW::getHash() const
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

    void BlockHeaderPoW::serialize(ByteBuffer& stream) const
    {
        stream << version;
        hashPrevBlock.serialize(stream);
        hashMerkleRoot.serialize(stream);
        stream << timeval << bits << proof;
    }
    
	Block::Block()
		: index_(0)
		, transactions_()
        , pBlockHeader_(new BlockHeaderPoW())
	{
	}

	Block::~Block()
	{
        SAFE_RELEASE(pBlockHeader_);
	}
}
