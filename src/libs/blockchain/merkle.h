// Copyright (c) 2015-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MERKLE
#define BITCOIN_MERKLE

#include <stdint.h>
#include <vector>

#include "transaction.h"
#include "block.h"
#include "common/uint256.h"

namespace P2pClouds {

	uint256_t ComputeMerkleRoot(const std::vector<uint256_t>& leaves, bool* mutated = nullptr);
	std::vector<uint256_t> ComputeMerkleBranch(const std::vector<uint256_t>& leaves, uint32_t position);
	uint256_t ComputeMerkleRootFromBranch(const uint256_t& leaf, const std::vector<uint256_t>& branch, uint32_t position);

	/*
	 * Compute the Merkle root of the transactions in a block.
	 * *mutated is set to true if a duplicated subtree was found.
	 */
	uint256_t BlockMerkleRoot(const Block& block, bool* mutated = nullptr);

	/*
	 * Compute the Merkle root of the witness transactions in a block.
	 * *mutated is set to true if a duplicated subtree was found.
	 */
	uint256_t BlockWitnessMerkleRoot(const Block& block, bool* mutated = nullptr);

	/*
	 * Compute the Merkle branch for the tree of transactions in a block, for a
	 * given position.
	 * This can be verified using ComputeMerkleRootFromBranch.
	 */
	std::vector<uint256_t> BlockMerkleBranch(const Block& block, uint32_t position);

}

#endif
