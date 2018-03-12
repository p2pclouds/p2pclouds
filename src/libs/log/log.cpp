#include "log.h"
#include "common/common.h"

namespace P2pClouds {

	static std::string _g_logFileName = "log.log";

	bool Log::configure(const std::string& logFileName)
	{
		_g_logFileName = logFileName;
		return true;
	}

	Log::Log():
		logger_()
	{
#if P2PCLOUDS_PLATFORM == PLATFORM_WIN32
        if (_access("logs", 0) == -1)
        {
			int flag = _mkdir("logs");
#else
        if (access("logs", 0) == -1)
        {
			int flag = mkdir("logs", 0777);
#endif    
			assert (flag == 0);
		}

		try 
		{
			std::vector<spdlog::sink_ptr> sinks;

			spdlog::set_async_mode(32768);

#ifdef LOG_TO_CONSOLE  
			sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
#endif

			sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>
				(fmt::format("logs/{}", _g_logFileName), 1 * 1024L * 1024, 1));

			logger_ = std::make_shared<spdlog::logger>("both", begin(sinks), end(sinks));
			spdlog::register_logger(logger_);

#ifdef _DEBUG  
			logger_->set_level(spdlog::level::debug);
#else  
			logger_->set_level(spdlog::level::debug);
#endif

			logger_->set_pattern("[%Y-%m-%d %H:%M:%S] %t %L %v");

			logger_->flush_on(spdlog::level::debug);
		}
		catch (const spdlog::spdlog_ex& ex)
		{
			std::cout << "Log init failed: " << ex.what() << std::endl;
			exit(0);
		}
	}

	Log::~Log()
	{
		spdlog::drop_all();
	}
}
