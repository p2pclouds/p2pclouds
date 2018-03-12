#include "common/common.h"
#include "log/log.h"
#include "test_app.h"

#include "common/arith_uint256.h"
DEFINE_uint64(id, 0, "the server id");
DEFINE_int32(numThreads, 0, "num threads");

int main(int argc, char *argv[])
{
	P2pClouds::Log::configure("test_app.log");
	LOG_INFO("\n-------------------------------------------------------------------");

	gflags::ParseCommandLineFlags(&argc, &argv, true);

	P2pClouds::TestApp app(FLAGS_id, FLAGS_numThreads);

	try
	{
		if (app.initialize())
		{
			app.run();
		}
		else
		{
			LOG_ERROR("App::initialize(): error!");
		}
	}
	catch (std::exception& e)
	{
		LOG_ERROR("App::run(): error! what={}", e.what());
	}

    if (!app.finalise())
        return -1;
    
	LOG_INFO("App shutdown!");
	return 0;
}
