#include "consensus.h"
#include "merkle.h"
#include "blockchain.h"
#include "common/byte_buffer.h"
#include "common/hash.h"

namespace P2pClouds {
    
	ConsensusArgsPtr ConsensusArgs::create(ARGS_TYPE type)
	{
		ConsensusArgsPtr pArgs = std::make_shared<ConsensusArgs>();

		if (type == NORMAL)
		{
			pArgs->p_difficulty_1_target = arith_uint256("0x0000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
			pArgs->b_difficulty_1_target = arith_uint256("0x0000FFFF00000000000000000000000000000000000000000000000000000000");
			pArgs->cycleBlockHeight = 2016;
			pArgs->cycleTimestamp = (14 * 24 * 60 * 60);
			pArgs->subsidyHalvingInterval = 210000;
			pArgs->valueUnit = 100000000;
			pArgs->hashBlockGenesis = uint256S("0");
		}
		else if (type == TEST)
		{
			pArgs->p_difficulty_1_target = arith_uint256("0x0000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
			pArgs->b_difficulty_1_target = arith_uint256("0x0000FFFF00000000000000000000000000000000000000000000000000000000");
			pArgs->cycleBlockHeight = 2016;
			pArgs->cycleTimestamp = (14 * 24 * 60 * 60);
			pArgs->subsidyHalvingInterval = 210000;
			pArgs->valueUnit = 100000000;
			pArgs->hashBlockGenesis = uint256S("0");
		}

		return pArgs;
	}

	Consensus::Consensus(Blockchain* pBlockchain, ConsensusArgsPtr args)
    : pArgs_(args)
	, pBlockchain_(pBlockchain)
	{
	}

	Consensus::~Consensus()
	{
	}

    ConsensusPow::ConsensusPow(Blockchain* pBlockchain, ConsensusArgsPtr args)
    : Consensus(pBlockchain, args)
    {
        createGenesisBlock();
    }
    
    ConsensusPow::~ConsensusPow()
    {
    }
    
    void ConsensusPow::createGenesisBlock()
	{
        if(pBlockchain()->chainManager()->activeChainHeight() > 0)
            return;

		BlockPtr pBlock = std::make_shared<Block>(new BlockHeader());
        BlockHeader* pBlockHeader = pBlock->pBlockHeader();
        
		pBlockHeader->timeval = (uint32_t)getAdjustedTime();
		pBlockHeader->proof = 0;
		pBlockHeader->hashPrevBlock = uint256S("0");
		pBlockHeader->bits = pArgs()->b_difficulty_1_target.getCompact();

		// coin base
		TransactionPtr pBaseTransaction = std::make_shared<Transaction>();
		pBaseTransaction->magic(0);
		pBaseTransaction->value(0);
		pBaseTransaction->recipient("0");
		pBaseTransaction->sender("0");
		pBlock->transactions().push_back(pBaseTransaction);

		// packing Transactions
		pBlock->addTransactions(pBlockchain()->currentTransactions());
		pBlockHeader->hashMerkleRoot = BlockMerkleRoot(*pBlock);

		pArgs()->hashBlockGenesis = pBlock->getHash();
		pBlockchain()->processNewBlock(pBlock);
	}

    BlockPtr ConsensusPow::createNewBlock(uint32_t bits, uint32_t proof, unsigned int extraProof, BlockIndex* pTipBlockIndex)
	{
		BlockPtr pBlock = std::make_shared<Block>(new BlockHeader());
		BlockHeader* pBlockHeader = pBlock->pBlockHeader();
        
		pBlockHeader->timeval = (uint32_t)getAdjustedTime();
		pBlockHeader->proof = proof;
		pBlockHeader->hashPrevBlock = *pTipBlockIndex->phashBlock;
		pBlockHeader->bits = (bits == 0 ? getNextWorkTarget(pBlock, pTipBlockIndex) : bits);

		// coin base
		TransactionPtr pBaseTransaction = std::make_shared<Transaction>();
		pBaseTransaction->magic(extraProof);
		pBaseTransaction->value(calculateSubsidyValue(pTipBlockIndex->height + 1/* curr block */) - pBlockchain()->userGas());
		pBaseTransaction->recipient(pBlockchain()->userHash());
		pBaseTransaction->sender("0");
		pBlock->transactions().push_back(pBaseTransaction);

		// packing Transactions
		pBlock->addTransactions(pBlockchain()->currentTransactions());
		pBlockHeader->hashMerkleRoot = BlockMerkleRoot(*pBlock);

		return pBlock;
	}

    bool ConsensusPow::validBlock(BlockPtr pBlock)
    {
        BlockHeader* pBlockHeader = pBlock->pBlockHeader();

        if(!validProofOfWork(pBlock->getHash(), pBlockHeader->bits))
        {
			LOG_ERROR("or hash({}) doesn't match nBits! bits={})", pBlock->getHash().toString(), pBlockHeader->bits);
            return false;
        }

        return true;
    }

    bool ConsensusPow::build()
    {
        //LOG_DEBUG("Starting build...");

        const int innerLoopCount = 0x10000;
        uint64_t maxTries = pow(2, 32) - 1;
        uint64_t tries = 0;
        unsigned int extraProof = 0;
        
        time_t startTimestamp = getTimeStamp();
        BlockPtr pFoundBlock;
        float difficulty = 1.f;

		ChainPtr& activeChain = pBlockchain()->chainManager()->activeChain();
        uint32_t chainHeight = activeChain->height();
		BlockIndex* pLastBlock = activeChain->tip();

        while (true)
        {
            if(chainHeight != activeChain->height())
            {
                //LOG_DEBUG("chainSize({}) != currchainHeight({}), build canceled!", chainHeight, pBlockchain()->chainHeight());
                return false;
            }

            BlockPtr pNewBlock = createNewBlock(0, 0, ++extraProof, pLastBlock);
            BlockHeader* pBlockHeader = pNewBlock->pBlockHeader();
            
            arith_uint256 target;
            target.setCompact(pBlockHeader->bits);
            
            difficulty = (float)(pArgs_->b_difficulty_1_target / target).getdouble();
            
            ByteBuffer stream;
			pBlockHeader->serialize(stream);
            
            int woffset = stream.wpos() - sizeof(pBlockHeader->proof);
            
            uint256_t hash2561;
            uint256_t hash2562;
            
            do
            {
                ++tries;
                ++pBlockHeader->proof;
                
                stream.wpos(woffset);
                stream << pBlockHeader->proof;
                SHA256(stream.data(), stream.length(), (unsigned char*)&hash2561);
                SHA256(hash2561.begin(), uint256_t::WIDTH, (unsigned char*)&hash2562);

            } while (tries < maxTries && pBlockHeader->proof < innerLoopCount && !validProofOfWork(hash2562, pBlockHeader->bits));
            
            if (tries == maxTries)
            {
                break;
            }
            
            if (pBlockHeader->proof >= innerLoopCount)
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
        
        BlockHeader* pBlockHeader = pFoundBlock->pBlockHeader();
        
        uint64_t proof = tries;
        float elapsedTime = float(getTimeStamp() - startTimestamp) / 1000.f;
        float hashPower = proof / elapsedTime;
        
		// Allow to put into candidate area
		//if (pBlockHeader->hashPrevBlock != activeChain->tipBlock()->getHash())
		//	return false;

		BlockIndex* pBlockIndex = pBlockchain()->processNewBlock(pFoundBlock);
        if(!pBlockIndex)
            return false;

        LOG_DEBUG("Success with proof: {}", proof);
		LOG_DEBUG("chainHeight:{}, rewardValue:{}", pBlockIndex->height, (pFoundBlock->transactions()[0]->value() / pArgs_->valueUnit));
        LOG_DEBUG("Elapsed Time: {} seconds", elapsedTime);
        LOG_DEBUG("Thread finds need {} Minutes", ((difficulty * pow(2, 256 - pArgs_->p_difficulty_1_target.bits())) / hashPower / 60));
        LOG_DEBUG("Hashing Power: {} hashes per second", hashPower);
        LOG_DEBUG("Difficulty: {} (bits: {})", difficulty, pBlockHeader->bits);
		LOG_DEBUG("Hash: {}", pBlockIndex->phashBlock->toString());
        LOG_DEBUG("");
		LOG_DEBUG("");
        return true;
    }
    
    bool ConsensusPow::validProofOfWork(const uint256_t& hash, uint32_t bits)
    {
        bool isNegative;
        bool isOverflow;
        
        arith_uint256 target;
        target.setCompact(bits, &isNegative, &isOverflow);
        
        // Check range
		if (isNegative || target == 0 || isOverflow || target > pArgs_->p_difficulty_1_target)
		{
			LOG_ERROR("bits below minimum work! bits={})", bits);
			return false;
		}

        arith_uint256 hashResult;
        uintToArith256(hashResult, hash);
        
        if (hashResult > target)
            return false;
        
        return true;
    }

    uint32_t ConsensusPow::getWorkTarget(BlockPtr pBlock)
    {
		BlockIndex* pPrevBlockIndex = pBlockchain()->chainManager()->findBlockIndex(pBlock->pBlockHeader()->hashPrevBlock);

        if((pPrevBlockIndex->height + 1/* curr block */) % pArgs_->cycleBlockHeight != 1)
        {
            if(!pPrevBlockIndex)
                return 0;
        }
        
		assert(pPrevBlockIndex);
        return getNextWorkTarget(pBlock, pPrevBlockIndex);
    }

    uint32_t ConsensusPow::getNextWorkTarget(BlockPtr pBlock, BlockIndex* pLastBlockIndex)
    {
		uint32_t currBlockHeight = (pLastBlockIndex->height + 1);
        if(currBlockHeight < pArgs_->cycleBlockHeight || currBlockHeight % pArgs_->cycleBlockHeight != 1)
        {
            return pLastBlockIndex->bits;
        }

        return calculateNextWorkTarget(pBlock, pLastBlockIndex);
    }

    uint32_t ConsensusPow::calculateNextWorkTarget(BlockPtr pBlock, BlockIndex* pLastBlockIndex)
    {
		BlockIndex* pFirstBlockIndex = pBlockchain()->chainManager()->getBlock(pLastBlockIndex->height, pArgs_->cycleBlockHeight);

        assert(pFirstBlockIndex && pLastBlockIndex);

        uint32_t diffTimestamp = pLastBlockIndex->getTimeval() - pFirstBlockIndex->getTimeval();

        if (diffTimestamp < pArgs_->cycleTimestamp / 4)
            diffTimestamp = pArgs_->cycleTimestamp / 4;

        if (diffTimestamp > pArgs_->cycleTimestamp * 4)
            diffTimestamp = pArgs_->cycleTimestamp * 4;

        arith_uint256 bnNew;
        bnNew.setCompact(pLastBlockIndex->bits);

        bnNew *= diffTimestamp;
        bnNew /= pArgs_->cycleTimestamp;

        if(bnNew > pArgs_->p_difficulty_1_target)
            bnNew = pArgs_->p_difficulty_1_target;

        return bnNew.getCompact();
    }

    uint64_t ConsensusPow::calculateSubsidyValue(uint32_t blockHeight)
    {
        int halvings = blockHeight / pArgs_->subsidyHalvingInterval;

        // Force block reward to zero when right shift is undefined.
        if (halvings >= 64)
            return 0;

        uint64_t subsidy = 50 * pArgs_->valueUnit;

        // Subsidy is cut in half every subsidyHalvingInterval blocks.
        subsidy >>= halvings;

        return subsidy;
    }
}
