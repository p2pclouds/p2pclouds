// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "uint256.h"

#include <stdio.h>
#include <string.h>

namespace P2pClouds {

	template<typename T>
	std::string hexStr(const T itbegin, const T itend, bool fSpaces = false)
	{
		std::string rv;
		static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
		rv.reserve((itend - itbegin) * 3);
		for (T it = itbegin; it < itend; ++it)
		{
			unsigned char val = (unsigned char)(*it);
			if (fSpaces && it != itbegin)
				rv.push_back(' ');
			rv.push_back(hexmap[val >> 4]);
			rv.push_back(hexmap[val & 15]);
		}

		return rv;
	}

	const signed char p_util_hexdigit[256] =
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,
	-1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, };

	signed char hexDigit(char c)
	{
		return p_util_hexdigit[(unsigned char)c];
	}

	template<typename T>
	inline std::string hexStr(const T& vch, bool fSpaces = false)
	{
		return hexStr(vch.begin(), vch.end(), fSpaces);
	}

	template <unsigned int BITS>
	base_blob<BITS>::base_blob(const std::vector<unsigned char>& vch)
	{
		assert(vch.size() == sizeof(data));
		memcpy(data, vch.data(), sizeof(data));
	}

	template <unsigned int BITS>
	std::string base_blob<BITS>::getHex() const
	{
		return hexStr(std::reverse_iterator<const uint8_t*>(data + sizeof(data)), std::reverse_iterator<const uint8_t*>(data));
	}

	template <unsigned int BITS>
	void base_blob<BITS>::setHex(const char* psz)
	{
		memset(data, 0, sizeof(data));

		// skip leading spaces
		while (isspace(*psz))
			psz++;

		// skip 0x
		if (psz[0] == '0' && tolower(psz[1]) == 'x')
			psz += 2;

		// hex string to uint
		const char* pbegin = psz;
		while (hexDigit(*psz) != -1)
			psz++;
		psz--;
		unsigned char* p1 = (unsigned char*)data;
		unsigned char* pend = p1 + WIDTH;
		while (psz >= pbegin && p1 < pend) {
			*p1 = hexDigit(*psz--);
			if (psz >= pbegin) {
				*p1 |= ((unsigned char)hexDigit(*psz--) << 4);
				p1++;
			}
		}
	}

	template <unsigned int BITS>
	void base_blob<BITS>::setHex(const std::string& str)
	{
		setHex(str.c_str());
	}

	template <unsigned int BITS>
	std::string base_blob<BITS>::toString() const
	{
		return (getHex());
	}

	// Explicit instantiations for base_blob<160>
	template base_blob<160>::base_blob(const std::vector<unsigned char>&);
	template std::string base_blob<160>::getHex() const;
	template std::string base_blob<160>::toString() const;
	template void base_blob<160>::setHex(const char*);
	template void base_blob<160>::setHex(const std::string&);

	// Explicit instantiations for base_blob<256>
	template base_blob<256>::base_blob(const std::vector<unsigned char>&);
	template std::string base_blob<256>::getHex() const;
	template std::string base_blob<256>::toString() const;
	template void base_blob<256>::setHex(const char*);
	template void base_blob<256>::setHex(const std::string&);

}
