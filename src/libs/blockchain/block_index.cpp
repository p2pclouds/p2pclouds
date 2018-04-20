#include "block_index.h"
#include "common/hash.h"

namespace P2pClouds {

	BlockIndex::BlockIndex(BlockPtr pBlock)
		: height(0)
		, chainWork()
		, status(VALID_UNKNOWN)
		, phashBlock(NULL)
		, pPrev(NULL)
		, version(P2PCLOUDS_VERSION)
		, hashMerkleRoot()
		, timeval(0)
		, bits(0)
		, proof(0)
		, sequenceID(0)
		, numBlockTransactions(0)
		, numChainTransactions(0)
	{
		version = pBlock->pBlockHeader()->version;
		hashMerkleRoot = pBlock->pBlockHeader()->hashMerkleRoot;
		timeval = pBlock->pBlockHeader()->timeval;
		bits = pBlock->pBlockHeader()->bits;
		proof = pBlock->pBlockHeader()->proof;
	}

	BlockIndex::~BlockIndex()
	{
	}

	std::string BlockIndex::toString()
	{
		return fmt::format("height={}, chainWork={}, status={}, sequenceID={}, numBlockTransactions={}, numChainTransactions={}",
			height, chainWork.toString(), status, sequenceID, numBlockTransactions, numChainTransactions);
	}

}
