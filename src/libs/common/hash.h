#pragma once

#include "common/byte_buffer.h"
#include <openssl/opensslv.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>

namespace P2pClouds {

	class Hash160
	{
	public:
		Hash160()
			: _ripemd160()
			, _hash()
			, _final(false)
		{
			RIPEMD160_Init(&_ripemd160);
		}

		~Hash160()
		{
			if (!_final)
            {
                 _final = true;
				RIPEMD160_Final((unsigned char*)&_hash, &_ripemd160);
            }
		}

		int update(const void *data, size_t len)
		{
			assert(!_final);
			return RIPEMD160_Update(&_ripemd160, data, len);
		}

		int update(const ByteBuffer& stream)
		{
			assert(!_final);
			return RIPEMD160_Update(&_ripemd160, stream.data(), stream.length());
		}

		int update(const std::string& stream)
		{
			assert(!_final);
			return RIPEMD160_Update(&_ripemd160, stream.data(), stream.length());
		}

		uint160_t& getHash()
		{
			if (!_final)
            {
                 _final = true;
				RIPEMD160_Final((unsigned char*)&_hash, &_ripemd160);
            }

			return _hash;
		}

	private:
		RIPEMD160_CTX _ripemd160;
		uint160_t _hash;
		bool _final;
	};

	class Hash256
	{
	public:
		Hash256()
			: _sha256()
			, _hash()
			, _final(false)
		{
			SHA256_Init(&_sha256);
		}

		~Hash256()
		{
			if (!_final)
            {
                 _final = true;
				SHA256_Final((unsigned char*)&_hash, &_sha256);
            }
		}

		int update(const void *data, size_t len)
		{
			assert(!_final);
			return SHA256_Update(&_sha256, data, len);
		}

		int update(const ByteBuffer& stream)
		{
			assert(!_final);
			return SHA256_Update(&_sha256, stream.data(), stream.length());
		}

		int update(const std::string& stream)
		{
			assert(!_final);
			return SHA256_Update(&_sha256, stream.data(), stream.length());
		}

		uint256_t& getHash()
		{
			if (!_final)
            {
                 _final = true;
				SHA256_Final((unsigned char*)&_hash, &_sha256);
            }

			return _hash;
		}

	private:
		SHA256_CTX _sha256;
		uint256_t _hash;
		bool _final;
	};

}
