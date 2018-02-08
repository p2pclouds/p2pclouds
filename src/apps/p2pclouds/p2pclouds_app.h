#pragma once

#include "app/app.h"

namespace P2pClouds {

	class P2pCloudsApp : public App
	{
	public:
		P2pCloudsApp();
		virtual ~P2pCloudsApp();

		bool initialize() override;
		bool finalise() override;

		bool run() override;

	protected:
		void netEventCallback(std::shared_ptr<Session> pSession, NetEventType event_type, ByteBuffer* pdatas) override;
	};

}
