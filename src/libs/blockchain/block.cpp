#include "block.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {

    void BlockHeader::serialize(ByteBuffer& stream) const
    {
        stream << version;
        hashPrevBlock.serialize(stream);
        hashMerkleRoot.serialize(stream);
        stream << timeval << bits << proof;
    }

	uint32_t BlockHeader::getSerializeSize()
	{
		uint32_t size = 0;

		size += ByteBuffer::typeSize(version);
		size += ByteBuffer::typeSize(hashPrevBlock);
		size += ByteBuffer::typeSize(hashMerkleRoot);
		size += ByteBuffer::typeSize(timeval);
		size += ByteBuffer::typeSize(bits);
		size += ByteBuffer::typeSize(proof);

		return size;
	}

	uint256_t BlockHeader::getHash() const
	{
		ByteBuffer stream;
        
        stream.clear(false);
        serialize(stream);

        uint256_t hash2561;
        SHA256(stream.data(), stream.length(), (unsigned char*)&hash2561);

        uint256_t hash2562;
        SHA256(hash2561.begin(), uint256_t::WIDTH, (unsigned char*)&hash2562);
		return hash2562;
	}

	std::string BlockHeader::toString()
	{
		return fmt::format("version={}, timeval={}, bits={}, proof={}, hash={}, hashPrevBlock={}, hashMerkleRoot={}",
			version, timeval, bits, proof, getHash().toString(), hashPrevBlock.toString(), hashMerkleRoot.toString());
	}

	Block::Block()
		: transactions_()
        , pBlockHeader_(new BlockHeader())
	{
	}

	Block::~Block()
	{
        SAFE_RELEASE(pBlockHeader_);
	}

	uint32_t Block::getSerializeSize()
	{
		uint32_t size = pBlockHeader_->getSerializeSize();

		for (auto& item : transactions_)
		{
			size += item->getSerializeSize();
		}

		return size;
	}

}
