#pragma once

#include "app/app.h"

namespace P2pClouds {

	class TestApp : public App
	{
	public:
		TestApp(uint64_t id, int32_t numThreads);
		virtual ~TestApp();

		bool initialize() override;
		bool initNetworkInterfaces() override;

		bool finalise() override;

		bool run() override;

	protected:
		void netEventCallback(std::shared_ptr<Session> pSession, NetEventType event_type, ByteBuffer* pdatas) override;
	};

}
