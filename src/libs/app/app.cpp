#include "app.h"
#include "network/network_interface.h"

namespace P2pClouds {

	App::App(uint64_t id, int32_t numThreads)
		: pNetworkInterface_(NULL)
		, ioService_()
		, signals_(ioService_)
        , id_(id)
        , numThreads_(numThreads)
	{
		doAwaitStop();
	}

	App::~App()
	{
		SAFE_RELEASE(pNetworkInterface_);
	}

	void App::doAwaitStop()
	{
		// Register to handle the signals that indicate when the server should exit.
		// It is safe to register for the same signal multiple times in a program,
		// provided all registration for the specified signal is made through Asio.
		signals_.add(SIGINT);
		signals_.add(SIGTERM);

#if defined(SIGQUIT)
		signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

		signals_.async_wait(
			[this](std::error_code /*ec*/, int /*signo*/)
		{
			// The server is stopped by cancelling all outstanding asynchronous
			// operations. Once all operations have finished the io_service::run()
			// call will exit.
			pNetworkInterface_->stopAll();
		});
	}

	bool App::initialize()
	{
		return initNetworkInterfaces();
	}

	bool App::initNetworkInterfaces()
	{
		pNetworkInterface_ = new NetworkInterface(ioService_, "127.0.0.1", 27776);
		pNetworkInterface_->setEventCallback(std::bind(&App::netEventCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		return pNetworkInterface_->initialize();
	}

	bool App::finalise()
	{
        if (pNetworkInterface_)
            pNetworkInterface_->stopAll();
        
		return true;
	}

	bool App::run()
	{
		// The io_service::run() call will block until all asynchronous operations
		// have finished. While the server is running, there is always at least one
		// asynchronous operation outstanding: the asynchronous accept call waiting
		// for new incoming connections.
		ioService_.run();
		return true;
	}

	void App::netEventCallback(std::shared_ptr<Session> pSession, NetEventType event_type, ByteBuffer* pdatas)
	{
	}
}
