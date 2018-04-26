#include "block_index.h"
#include "chain.h"
#include "common/hash.h"

namespace P2pClouds {

	BlockIndex::BlockIndex(BlockPtr pBlock)
		: height(0)
		, chainWork()
		, status(VALID_UNKNOWN)
		, phashBlock(NULL)
		, pPrev(NULL)
		, pSkip(NULL)
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

	BlockIndex* BlockIndex::getAncestor(int inputHeight)
	{
		if (inputHeight > height || inputHeight < 0)
			return NULL;

		// First set the walk position to yourself
		BlockIndex* pIndexWalk = this;

		int heightWalk = height;

		/*
				    ancestorPos <- inputPos                             <- thisPos
					       |         |                                       |
			|   -1-   |   -2-   |   -3-   |   -4-   |   -5-   |   -6-   |   -7-   |   -8-   |   -9-   |
		*/

		// Its height is less than the input height
		while (heightWalk > inputHeight)
		{
			int heightSkip = ChainSkipList::getSkipHeight(heightWalk);
			int heightSkipPrev = ChainSkipList::getSkipHeight(heightWalk - 1);

			if (heightSkip == inputHeight ||
				(heightSkip > inputHeight && !(heightSkipPrev < heightSkip - 2 &&
				heightSkipPrev >= inputHeight)))
			{
				// Only follow pskip if pprev->pskip isn't better than pskip->pprev.
				pIndexWalk = pIndexWalk->pSkip;
				heightWalk = heightSkip;
			}
			else 
			{
				pIndexWalk = pIndexWalk->pPrev;
				heightWalk--;
			}
		}

		return pIndexWalk;
	}

	void BlockIndex::buildSkip()
	{
		// pSkip is prevBlockIndex
		if (pPrev)
			pSkip = pPrev->getAncestor(ChainSkipList::getSkipHeight(height));
	}
}
