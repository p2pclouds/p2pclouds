#include "consensus.h"
#include "merkle.h"
#include "blockchain.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {
    
	Consensus::Consensus(Blockchain* pBlockchain)
    : pBlockchain_(pBlockchain)
	{
	}

	Consensus::~Consensus()
	{
	}

    bool Consensus::validBlock(BlockPtr pBlock)
    {
        BlockHeader* pBlockHeader = (BlockHeader*)pBlock->pBlockHeader();

        if(!validBlockTime(pBlockHeader->timeval))
            return false;

        if(!pBlockchain()->getPrevBlock(pBlock))
            return false;
    
        return true;
    }

    bool Consensus::validBlockTime(time_t timeval)
    {
        if (timeval > getAdjustedTime() + 2 * 60 * 60)
        {
            LOG_ERROR("Illegal timeval({}), not conforming to adjustedTime({})!", timeval, getAdjustedTime());
            return false;
        }

        time_t medianTime = pBlockchain()->getMedianBlockTimePastInChain();
        if(timeval < medianTime)
        {
            LOG_ERROR("Illegal timeval({}), not conforming to medianTime({})!", timeval, medianTime);
            return false;
        }

        return true;
    }

    arith_uint256 ConsensusPow::p_difficulty_1_target("0x0000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    arith_uint256 ConsensusPow::b_difficulty_1_target("0x0000FFFF00000000000000000000000000000000000000000000000000000000");
    uint32_t ConsensusPow::cycleBlockHeight = 2016;
    uint32_t ConsensusPow::cycleTimestamp = (14 * 24 * 60 * 60);
    uint32_t ConsensusPow::subsidyHalvingInterval = 210000;
    uint64_t ConsensusPow::valueUnit = 100000000;

    ConsensusPow::ConsensusPow(Blockchain* pBlockchain)
    : Consensus(pBlockchain)
    {
        createGenesisBlock();
    }
    
    ConsensusPow::~ConsensusPow()
    {
    }
    
    void ConsensusPow::createGenesisBlock()
	{
        if(pBlockchain()->chainHeight() > 0)
            return;

		BlockPtr pBlock = std::make_shared<Block>(new BlockHeaderPoW());
        BlockHeaderPoW* pBlockHeaderPoW = (BlockHeaderPoW*)pBlock->pBlockHeader();
        
		pBlock->height(1);
		pBlockHeaderPoW->timeval = (uint32_t)getAdjustedTime();
		pBlockHeaderPoW->proof = 0;
		pBlockHeaderPoW->hashPrevBlock = uint256S("0");
        pBlockHeaderPoW->bits = b_difficulty_1_target.getCompact();

		// coin base
		TransactionPtr pBaseTransaction = std::make_shared<Transaction>();
		pBaseTransaction->magic(0);
		pBaseTransaction->value(0);
		pBaseTransaction->recipient("0");
		pBaseTransaction->sender("0");
		pBlock->transactions().push_back(pBaseTransaction);

		// packing Transactions
		pBlock->addTransactions(pBlockchain()->currentTransactions());
        pBlockHeaderPoW->hashMerkleRoot = BlockMerkleRoot(*pBlock);

		pBlockchain()->addBlockToChain(pBlock);
	}

    BlockPtr ConsensusPow::createNewBlock(uint32_t bits, uint32_t proof, unsigned int extraProof, BlockPtr pLastBlock, bool pushToChain)
	{
		BlockPtr pBlock = std::make_shared<Block>(new BlockHeaderPoW());
        BlockHeaderPoW* pBlockHeaderPoW = (BlockHeaderPoW*)pBlock->pBlockHeader();
        
		pBlock->height(pBlockchain()->chainHeight() + 1);
		pBlockHeaderPoW->timeval = (uint32_t)getAdjustedTime();
		pBlockHeaderPoW->proof = proof;
		pBlockHeaderPoW->hashPrevBlock = pLastBlock->getHash();
        pBlockHeaderPoW->bits = (bits == 0 ? getNextWorkTarget(pBlock, pLastBlock) : bits);

		// coin base
		TransactionPtr pBaseTransaction = std::make_shared<Transaction>();
		pBaseTransaction->magic(extraProof);
		pBaseTransaction->value(calculateSubsidyValue(pBlock->height()));
		pBaseTransaction->recipient(pBlockchain()->userHash());
		pBaseTransaction->sender("0");
		pBlock->transactions().push_back(pBaseTransaction);

		// packing Transactions
		pBlock->addTransactions(pBlockchain()->currentTransactions());
        pBlockHeaderPoW->hashMerkleRoot = BlockMerkleRoot(*pBlock);
		
		if (pushToChain)
			pBlockchain()->addBlockToChain(pBlock);

		return pBlock;
	}

    bool ConsensusPow::validBlock(BlockPtr pBlock)
    {
        BlockHeaderPoW* pBlockHeaderPoW = (BlockHeaderPoW*)pBlock->pBlockHeader();
        if(getWorkTarget(pBlock) != pBlockHeaderPoW->bits)
        {
            LOG_ERROR("Difficulty mismatch! Block({}) != {})", pBlockHeaderPoW->bits, getWorkTarget(pBlock));
            return false;
        }

        return Consensus::validBlock(pBlock);
    }

    bool ConsensusPow::validBlockTime(time_t timeval)
    {
        return Consensus::validBlockTime(timeval);
    }

    bool ConsensusPow::build()
    {
        LOG_DEBUG("Starting build...");

        const int innerLoopCount = 0x10000;
        uint64_t maxTries = pow(2, 32) - 1;
        uint64_t tries = 0;
        unsigned int extraProof = 0;
        
        time_t startTimestamp = getTimeStamp();
        BlockPtr pLastBlock = pBlockchain()->lastBlock();
        BlockPtr pFoundBlock;
        float difficulty = 1.f;
        uint32_t chainHeight = pBlockchain()->chainHeight();

        while (true)
        {
            if(chainHeight != pBlockchain()->chainHeight())
            {
                LOG_DEBUG("chainSize({}) != currchainHeight({}), build canceled!", chainHeight, pBlockchain()->chainHeight());
                return false;
            }

            BlockPtr pNewBlock = createNewBlock(0, 0, ++extraProof, pLastBlock, false);
            BlockHeaderPoW* pBlockHeaderPoW = (BlockHeaderPoW*)pNewBlock->pBlockHeader();
            
            arith_uint256 target;
            target.setCompact(pBlockHeaderPoW->bits);
            
            difficulty = (float)(b_difficulty_1_target / target).getdouble();
            
            ByteBuffer stream;
            pBlockHeaderPoW->serialize(stream);
            
            int woffset = stream.wpos() - sizeof(pBlockHeaderPoW->proof);
            
            uint256_t hash2561;
            uint256_t hash2562;
            
            do
            {
                ++tries;
                ++pBlockHeaderPoW->proof;
                
                stream.wpos(woffset);
                stream << pBlockHeaderPoW->proof;
                SHA256(stream.data(), stream.length(), (unsigned char*)&hash2561);
                SHA256(hash2561.begin(), uint256::WIDTH, (unsigned char*)&hash2562);

            } while (tries < maxTries && pBlockHeaderPoW->proof < innerLoopCount && !validProofOfWork(hash2562, pBlockHeaderPoW->proof, pBlockHeaderPoW->bits));
            
            if (tries == maxTries)
            {
                break;
            }
            
            if (pBlockHeaderPoW->proof >= innerLoopCount)
            {
                //LOG_ERROR("Failed after {} (maxProof) tries)", pBlockHeaderPoW->proof);
                //LOG_DEBUG("");
                continue;
            }
            
            pFoundBlock = pNewBlock;
            break;
        }
        
        if (!pFoundBlock)
        {
            LOG_ERROR("Failed to proof of work! tries={})", tries);
            LOG_DEBUG("");
            return false;
        }
        
        BlockHeaderPoW* pBlockHeaderPoW = (BlockHeaderPoW*)pFoundBlock->pBlockHeader();
        
        uint64_t proof = tries;
        float elapsedTime = float(getTimeStamp() - startTimestamp) / 1000.f;
        float hashPower = proof / elapsedTime;
        
    	if(pBlockHeaderPoW->hashPrevBlock != pBlockchain()->lastBlock()->getHash())
			return false;

        if(!pBlockchain()->addBlockToChain(pFoundBlock))
            return false;
        
        LOG_DEBUG("Success with proof: {}, chainHeight:{}, rewardValue:{}", proof, pFoundBlock->height(), (pFoundBlock->transactions()[0]->value() / valueUnit));
        LOG_DEBUG("Hash: {}", pFoundBlock->getHash().toString());
        LOG_DEBUG("Elapsed Time: {} seconds", elapsedTime);
        LOG_DEBUG("Current thread finds a hash need {} Minutes", ((difficulty * pow(2, 256 - p_difficulty_1_target.bits())) / hashPower / 60));
        LOG_DEBUG("Hashing Power: {} hashes per second", hashPower);
        LOG_DEBUG("Difficulty: {} (bits: {})", difficulty, pBlockHeaderPoW->bits);
        LOG_DEBUG("");
        return true;
    }
    
    bool ConsensusPow::validProofOfWork(const uint256_t& hash, uint32_t proof, uint32_t bits)
    {
        bool isNegative;
        bool isOverflow;
        
        arith_uint256 target;
        target.setCompact(bits, &isNegative, &isOverflow);
        
        // Check range
        if (isNegative || target == 0 || isOverflow || target > p_difficulty_1_target)
            return false;
        
        arith_uint256 hashResult;
        uintToArith256(hashResult, hash);
        
        if (hashResult > target)
            return false;
        
        return true;
    }

    uint32_t ConsensusPow::getWorkTarget(BlockPtr pBlock)
    {
        if(pBlock->height() % ConsensusPow::cycleBlockHeight != 1)
        {
            BlockPtr pPrevBlock = pBlockchain()->getPrevBlock(pBlock);
            if(!pPrevBlock)
                return 0;
        }
        
        return getNextWorkTarget(pBlock, pBlockchain()->getPrevBlock(pBlock));
    }

    uint32_t ConsensusPow::getNextWorkTarget(BlockPtr pBlock, BlockPtr pLastBlock)
    {
        if(pBlock->height() < ConsensusPow::cycleBlockHeight || pBlock->height() % ConsensusPow::cycleBlockHeight != 1)
        {
            return ((BlockHeaderPoW*)pLastBlock->pBlockHeader())->bits;
        }

        return calculateNextWorkTarget(pBlock, pLastBlock);
    }

    uint32_t ConsensusPow::calculateNextWorkTarget(BlockPtr pBlock, BlockPtr pLastBlock)
    {
        BlockPtr pFirstBlock = pBlockchain()->getBlock(pLastBlock->height(), ConsensusPow::cycleBlockHeight);

        assert(pFirstBlock.get() && pLastBlock.get());

        BlockHeaderPoW* pFirstBlockHeaderPoW = (BlockHeaderPoW*)pFirstBlock->pBlockHeader();
        BlockHeaderPoW* pLastBlockHeaderPoW = (BlockHeaderPoW*)pLastBlock->pBlockHeader();

        uint32_t diffTimestamp = pLastBlockHeaderPoW->getTimeval() - pFirstBlockHeaderPoW->getTimeval();

        if (diffTimestamp < ConsensusPow::cycleTimestamp / 4)
            diffTimestamp = ConsensusPow::cycleTimestamp / 4;

        if (diffTimestamp > ConsensusPow::cycleTimestamp * 4)
            diffTimestamp = ConsensusPow::cycleTimestamp * 4;

        arith_uint256 bnNew;
        bnNew.setCompact(pLastBlockHeaderPoW->bits);

        bnNew *= diffTimestamp;
        bnNew /= cycleTimestamp;

        if(bnNew > p_difficulty_1_target)
            bnNew = p_difficulty_1_target;

        return bnNew.getCompact();
    }

    uint64_t ConsensusPow::calculateSubsidyValue(uint32_t blockHeight)
    {
        int halvings = blockHeight / subsidyHalvingInterval;

        // Force block reward to zero when right shift is undefined.
        if (halvings >= 64)
            return 0;

        uint64_t subsidy = 50 * valueUnit;

        // Subsidy is cut in half every subsidyHalvingInterval blocks.
        subsidy >>= halvings;

        return subsidy;
    }
}
