#pragma once

#define PLATFORM_WIN32              0
#define PLATFORM_UNIX               1
#define PLATFORM_APPLE              2

#define UNIX_FLAVOUR_LINUX          1
#define UNIX_FLAVOUR_BSD            2
#define UNIX_FLAVOUR_OTHER          3
#define UNIX_FLAVOUR_OSX            4

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define P2PCLOUDS_PLATFORM PLATFORM_WIN32
#elif defined( __INTEL_COMPILER )
#  define P2PCLOUDS_PLATFORM PLATFORM_INTEL
#elif defined( __APPLE_CC__ )
#  define P2PCLOUDS_PLATFORM PLATFORM_APPLE
#else
#  define P2PCLOUDS_PLATFORM PLATFORM_UNIX
#endif

#define COMPILER_MICROSOFT 0
#define COMPILER_GNU       1
#define COMPILER_BORLAND   2
#define COMPILER_INTEL     3
#define COMPILER_CLANG     4

#ifdef _MSC_VER
#  define P2PCLOUDS_COMPILER COMPILER_MICROSOFT
#elif defined( __INTEL_COMPILER )
#  define P2PCLOUDS_COMPILER COMPILER_INTEL
#elif defined( __BORLANDC__ )
#  define P2PCLOUDS_COMPILER COMPILER_BORLAND
#elif defined( __GNUC__ )
#  define P2PCLOUDS_COMPILER COMPILER_GNU
#elif defined( __clang__ )
#  define P2PCLOUDS_COMPILER COMPILER_CLANG

#else
#  pragma error "FATAL ERROR: Unknown compiler."
#endif

// common include	
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <math.h>
#include <assert.h> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>  
#include <cstring>  
#include <vector>
#include <map>
#include <list>
#include <set>
#include <deque>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>
#include <cctype>
#include <iterator>
#include <random>
#include <chrono>
#include <condition_variable>
#include <future>

#include "common/version.h"
#include "common/singleton.h"
#include "common/uint256.h"
#include "common/arith_uint256.h"

#include "utf8.h"
#include <gflags/gflags.h>
#include <asio.hpp>

#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#if P2PCLOUDS_PLATFORM == PLATFORM_WIN32
#pragma warning(disable:4996)
#pragma warning(disable:4819)
#pragma warning(disable:4049)
#pragma warning(disable:4217)
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Rpcrt4.lib")
#include <io.h>
#include <time.h> 
//#define FD_SETSIZE 1024
#ifndef WIN32_LEAN_AND_MEAN 
#include <winsock2.h>	
#include <mswsock.h> 
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> 
#include <unordered_map>
#include <functional>
#include <memory>
#include <io.h>  
#include <direct.h>
#define _SCL_SECURE_NO_WARNINGS
#else
// linux include
#include <errno.h>
#include <float.h>
#include <pthread.h>	
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <iconv.h>
#include <langinfo.h>   /* CODESET */
#include <stdint.h>
#include <signal.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <functional>
#include <memory>
#if P2PCLOUDS_PLATFORM == PLATFORM_APPLE
#else
#include <linux/types.h>
#include <linux/errqueue.h>
#include <linux/limits.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/resource.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <signal.h>

#if !defined( _WIN32 )
# include <pwd.h>
#else
#endif

namespace P2pClouds {

#ifndef TCHAR
#ifdef _UNICODE
	typedef wchar_t												    TCHAR;
#else
	typedef char											    	TCHAR;
#endif
#endif

	typedef uint160													uint160_t;
	typedef uint256													uint256_t;

    // Common buff size, such as: path、name、IP
    #define MAX_BUF													256


    #define SAFE_RELEASE(i)	\
	if (i) \
		{ \
			delete i; \
			i = NULL; \
		}

    #define SAFE_RELEASE_ARRAY(i) \
	if (i) \
		{ \
			delete[] i; \
			i = NULL; \
		}

	// Returns the high word of a long.
#define HIWORDL(b)		(((b) & 0xffff0000L) >> 16)
	// Returns the low word of a long.
#define LOWORDL(b)		( (b) & 0xffffL)

#if P2PCLOUDS_COMPILER != COMPILER_MICROSOFT
	#define p2pclouds_isnan isnan
	#define p2pclouds_isinf isinf
	#define p2pclouds_snprintf snprintf
	#define p2pclouds_vsnprintf vsnprintf
	#define p2pclouds_vsnwprintf vsnwprintf
	#define p2pclouds_snwprintf swprintf
	#define p2pclouds_stricmp strcasecmp
	#define p2pclouds_strnicmp strncasecmp
	#define p2pclouds_fileno fileno
	#define p2pclouds_va_copy va_copy
#else
	#define p2pclouds_isnan _isnan
	#define p2pclouds_isinf(x) (!_finite(x) && !_isnan(x))
	#define p2pclouds_snprintf _snprintf
	#define p2pclouds_vsnprintf _vsnprintf
	#define p2pclouds_vsnwprintf _vsnwprintf
	#define p2pclouds_snwprintf _snwprintf
	#define p2pclouds_stricmp _stricmp
	#define p2pclouds_strnicmp _strnicmp
	#define p2pclouds_fileno _fileno
	#define p2pclouds_va_copy( dst, src) dst = src

	#define strtoq   _strtoi64
	#define strtouq  _strtoui64
	#define strtoll  _strtoi64
	#define strtoull _strtoui64
	#define atoll    _atoi64

#endif // unix

	inline int p2pclouds_replace(std::string& str, const std::string& pattern, const std::string& newpat)
	{
		int count = 0;
		const size_t nsize = newpat.size();
		const size_t psize = pattern.size();

		for (size_t pos = str.find(pattern, 0);
			pos != std::string::npos;
			pos = str.find(pattern, pos + nsize))
		{
			str.replace(pos, psize, newpat);
			count++;
		}

		return count;
	}

	template<typename T>
	inline void p2pclouds_split(const std::basic_string<T>& s, T c, std::vector< std::basic_string<T> > &v)
	{
		v.clear();

		if (s.size() == 0)
			return;

		typename std::basic_string< T >::size_type i = 0;
		typename std::basic_string< T >::size_type j = s.find(c);

		while (j != std::basic_string<T>::npos)
		{
			std::basic_string<T> buf = s.substr(i, j - i);

			if (buf.size() > 0)
				v.push_back(buf);

			i = ++j; j = s.find(c, j);
		}

		if (j == std::basic_string<T>::npos)
		{
			std::basic_string<T> buf = s.substr(i, s.length() - i);
			if (buf.size() > 0)
				v.push_back(buf);
		}
	}

	inline void p2pclouds_split(const std::string& s, const std::string& c, std::vector< std::string > &v, const bool keep_empty = true)
	{
		v.clear();

		if (c.empty()) {
			v.push_back(s);
			return;
		}

		std::string::const_iterator substart = s.begin(), subend;

		while (true) {
			subend = std::search(substart, s.end(), c.begin(), c.end());
			std::string temp(substart, subend);
			if (keep_empty || !temp.empty()) {
				v.push_back(temp);
			}
			if (subend == s.end()) {
				break;
			}
			substart = subend + c.size();
		}
	}

	inline time_t getTimeStamp()
	{
		std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
			std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

		auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());

		//std::time_t timestamp = std::chrono::system_clock::to_time_t(tp);  
		return tmp.count();
	}

	inline time_t getSysTime()
	{
		return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	}
	
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

#if P2PCLOUDS_PLATFORM == PLATFORM_WIN32
	inline void setenv(const std::string& name, const std::string& value, int overwrite)
	{
		_putenv_s(name.c_str(), value.c_str());
	}
#else
	// Linux is setenv
#endif

	inline char* wchar2char(const wchar_t* ts, size_t* outlen = NULL)
	{
		int len = (int)((wcslen(ts) + 1) * sizeof(wchar_t));
		char* ccattr = (char *)malloc(len);
		memset(ccattr, 0, len);

		size_t slen = wcstombs(ccattr, ts, len);

		if (outlen)
		{
			if ((size_t)-1 != slen)
				*outlen = slen;
			else
				*outlen = 0;
		}

		return ccattr;
	};

	inline wchar_t* char2wchar(const char* cs, size_t* outlen = NULL)
	{
		int len = (int)((strlen(cs) + 1) * sizeof(wchar_t));
		wchar_t* ccattr = (wchar_t *)malloc(len);
		memset(ccattr, 0, len);

		size_t slen = mbstowcs(ccattr, cs, len);

		if (outlen)
		{
			if ((size_t)-1 != slen)
				*outlen = slen;
			else
				*outlen = 0;
		}

		return ccattr;
	};

	inline uint64_t uuid()
	{
		static int workid = 1;
		static int seqid = 0;
		static uint64_t last_stamp = getTimeStamp();

		uint64_t uniqueId = 0;
		uint64_t nowtime = getTimeStamp();
		uniqueId = nowtime << 22;
		uniqueId |= (workid & 0x3ff) << 12;

		if (nowtime < last_stamp)
		{
			printf("uuid(): error!\n");
			assert(false);
		}
		if (nowtime == last_stamp)
		{
			if (seqid++ == 0x1000)
			{
				do {
					nowtime = getTimeStamp();
				} while (nowtime <= last_stamp);

				seqid = 0;
			}
		}
		else
		{
			seqid = 0;
		}

		last_stamp = nowtime;

		uniqueId |= seqid & 0xFFF;

		return uniqueId;
	}

}
