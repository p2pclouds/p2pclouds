#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <iostream> 
#include <algorithm> 

namespace P2pClouds {

    #define P2PCLOUDS_LITTLE_ENDIAN								0
    #define P2PCLOUDS_BIG_ENDIAN								1

    #if !defined(P2PCLOUDS_ENDIAN)
    #  if defined (USE_BIG_ENDIAN)
    #    define P2PCLOUDS_ENDIAN P2PCLOUDS_BIG_ENDIAN
    #  else
    #    define P2PCLOUDS_ENDIAN P2PCLOUDS_LITTLE_ENDIAN
    #  endif
    #endif

	inline bool isPlatformLittleEndian()
	{
		int n = 1;
		return *((char*)&n) ? true : false;
	}

	namespace PlatformLittleEndianConverter
	{
		template<size_t T>
		inline void convert(char *val)
		{
			std::swap(*val, *(val + T - 1));
			convert<T - 2>(val + 1);
		}

		template<> inline void convert<0>(char *) {}
		template<> inline void convert<1>(char *) {}            // ignore central byte

		template<typename T> inline void apply(T *val)
		{
			convert<sizeof(T)>((char *)(val));
		}

		inline void convert(char *val, size_t size)
		{
			if (size < 2)
				return;

			std::swap(*val, *(val + size - 1));
			convert(val + 1, size - 2);
		}
	}

#if P2PCLOUDS_ENDIAN == P2PCLOUDS_BIG_ENDIAN
	template<typename T> inline void EndianConvert(T& val) { PlatformLittleEndianConverter::apply<T>(&val); }
	template<typename T> inline void EndianConvertReverse(T&) { }
#else
	template<typename T> inline void EndianConvert(T&) { }
	template<typename T> inline void EndianConvertReverse(T& val) { PlatformLittleEndianConverter::apply<T>(&val); }
#endif

	template<typename T> void EndianConvert(T*);         // will generate link error
	template<typename T> void EndianConvertReverse(T*);  // will generate link error

	inline void EndianConvert(uint8_t&) { }
	inline void EndianConvert(int8_t&) { }
	inline void EndianConvertReverse(uint8_t&) { }
	inline void EndianConvertReverse(int8_t&) { }

}
