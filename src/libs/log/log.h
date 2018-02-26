#pragma once

#include "common/singleton.h"
#include <spdlog/spdlog.h>  

namespace P2pClouds {

	class Log
	{
	public:
		Log();
		virtual ~Log();

		inline auto getLogger() { return logger_; }

		static bool configure(const std::string& logFileName);

	protected:
		std::shared_ptr<spdlog::logger> logger_;
	};

    #define LOG_TRACE(msg, ...) P2pClouds::Singleton<P2pClouds::Log>::instance()->getLogger()->trace(msg"  -> {}:{}()#{}.", ##__VA_ARGS__, __FILE__, __func__, __LINE__)
    #define LOG_DEBUG(msg, ...) P2pClouds::Singleton<P2pClouds::Log>::instance()->getLogger()->debug(msg, ##__VA_ARGS__)
	#define LOG_ERROR(msg, ...) P2pClouds::Singleton<P2pClouds::Log>::instance()->getLogger()->error(msg"  -> {}:{}()#{}.", ##__VA_ARGS__, __FILE__, __func__, __LINE__)
	#define LOG_WARNING(msg, ...) P2pClouds::Singleton<P2pClouds::Log>::instance()->getLogger()->debug(msg, ##__VA_ARGS__)
	#define LOG_INFO(msg, ...) P2pClouds::Singleton<P2pClouds::Log>::instance()->getLogger()->debug(msg, ##__VA_ARGS__)
	#define LOG_CRITICAL(msg, ...) P2pClouds::Singleton<P2pClouds::Log>::instance()->getLogger()->critical(msg"  -> {}:{}()#{}.", ##__VA_ARGS__, __FILE__, __func__, __LINE__); assert(false)

}
