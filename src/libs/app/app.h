#pragma once

#include "common/common.h"
#include "network/common.h"

namespace P2pClouds {

	class NetworkInterface;

	class App
	{
	public:
		App(uint64_t id, int32_t numThreads);
		virtual ~App();

		virtual bool initialize();
		virtual bool initNetworkInterfaces();

		virtual bool finalise();

		virtual bool run();

        uint64_t id() const {
            return id_;
        }
        
        int32_t numThreads() const
        {
            return numThreads_;
        }
        
	protected:
		// Wait for a request to stop the server.
		virtual void doAwaitStop();

		virtual void netEventCallback(std::shared_ptr<Session> pSession, NetEventType event_type, ByteBuffer* pdatas);

	protected:
		NetworkInterface* pNetworkInterface_;
		asio::io_service ioService_;

		// The signal_set is used to register for process termination notifications.
		asio::signal_set signals_;
        
        uint64_t id_;
        int32_t numThreads_;
	};

}
