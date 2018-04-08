#include "block.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {

    void BlockHeader::serialize(ByteBuffer& stream) const
    {
        stream << version;
        hashPrevBlock.serialize(stream);
        hashMerkleRoot.serialize(stream);
        stream << timeval;
    }

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
        BlockHeader::serialize(stream);
        stream << bits << proof;
    }
    
	Block::Block()
		: height_(0)
		, chainWork_()
		, transactions_()
        , pBlockHeader_(new BlockHeaderPoW())
	{
	}

	Block::~Block()
	{
        SAFE_RELEASE(pBlockHeader_);
	}
}
