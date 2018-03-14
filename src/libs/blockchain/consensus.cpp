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

    arith_uint256 ConsensusPow::p_difficulty_1_target("0x00000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    arith_uint256 ConsensusPow::b_difficulty_1_target("0x00000000FFFF0000000000000000000000000000000000000000000000000000");
    
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
		createNewBlock(0, 0, uint256S("1"));
	}

    BlockPtr ConsensusPow::createNewBlock(uint32_t proof, unsigned int extraProof, const uint256_t& hashPrevBlock, bool pushToChain)
	{
		BlockPtr pBlock = std::make_shared<Block>(new BlockHeaderPoW());
        BlockHeaderPoW* pBlockHeaderPoW = (BlockHeaderPoW*)pBlock->pBlockHeader();
        
		pBlock->index(pBlockchain()->chainSize() + 1);
		pBlockHeaderPoW->timestamp = (uint32_t)(getTimeStamp() & 0xfffffffful);
		pBlockHeaderPoW->proof = proof;
		pBlockHeaderPoW->hashPrevBlock = hashPrevBlock.size() ? hashPrevBlock : pBlockchain()->lastBlock()->getHash();

		// coin base
		TransactionPtr pBaseTransaction = std::make_shared<Transaction>();
		pBaseTransaction->proof(extraProof);
		pBaseTransaction->amount(0);
		pBaseTransaction->recipient("0");
		pBaseTransaction->sender("0");
		pBlock->transactions().push_back(pBaseTransaction);

		// packing Transactions
		pBlock->addTransactions(pBlockchain()->currentTransactions());
        pBlockHeaderPoW->hashMerkleRoot = BlockMerkleRoot(*pBlock);
		
		if (pushToChain)
			pBlockchain()->addBlockToChain(pBlock);

		return pBlock;
	}

    bool ConsensusPow::build()
    {
        LOG_DEBUG("Starting build...");
        
        const int innerLoopCount = 0x10000;
        uint64_t maxTries = pow(2, 32) - 1;
        uint64_t tries = 0;
        unsigned int extraProof = 0;
        
        time_t start_timestamp = getTimeStamp();
        BlockPtr pLastblock = pBlockchain()->lastBlock();
        BlockPtr pFoundBlock;
        float difficulty = 1.f;
        uint32_t chainSize = pBlockchain()->chainSize();

        while (true)
        {
            if(chainSize != pBlockchain()->chainSize())
            {
                LOG_DEBUG("chainSize({}) != currchainSize({}), build canceled!");
                return false;
            }

            BlockPtr pNewBlock = createNewBlock(0, ++extraProof, pLastblock->getHash(), false);
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
        float elapsedTime = float(getTimeStamp() - start_timestamp) / 1000.f;
        float hashPower = proof / elapsedTime;
        
    	if(pBlockHeaderPoW->hashPrevBlock != pBlockchain()->lastBlock()->getHash())
			return false;

        if(!pBlockchain()->addBlockToChain(pFoundBlock))
            return false;
        
        LOG_DEBUG("Success with proof: {}, chainHeight:{}", proof, pFoundBlock->index());
        LOG_DEBUG("Hash: {}", pFoundBlock->getHash().toString());
        LOG_DEBUG("Elapsed Time: {} seconds", elapsedTime);
        LOG_DEBUG("Current thread finds a hash need {} Minutes", ((difficulty * pow(2,32)) / hashPower / 60));
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
}
