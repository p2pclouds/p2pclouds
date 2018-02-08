#include "test_app.h"
#include "network/session.h"
#include "network/network_interface.h"
#include "log/log.h"

namespace P2pClouds {

	TestApp::TestApp()
		: App()
	{
	}

	TestApp::~TestApp()
	{
	}

	bool TestApp::initialize()
	{
		bool ret = App::initialize();

		for(int i=0; i<64; i++)
			pNetworkInterface_->connect("127.0.0.1");

		return ret;
	}

	bool TestApp::initNetworkInterfaces()
	{
		pNetworkInterface_ = new NetworkInterface(ioService_, "127.0.0.1", LISTEN_PORT + 1);
		pNetworkInterface_->setEventCallback(std::bind(&TestApp::netEventCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		return pNetworkInterface_->initialize();
	}

	bool TestApp::finalise()
	{
		return App::finalise();
	}

	bool TestApp::run()
	{
		return App::run();
	}

	void TestApp::netEventCallback(std::shared_ptr<Session> pSession, NetEventType event_type, ByteBuffer* pdatas) 
	{
		LOG_TRACE("netEventCallback: sessionID:{} type: {}", pSession->id(), netEventType2Str(event_type));
		if (event_type == NetRcvMsg)
		{
			std::string msg;
			(*pdatas) >> msg;
			static int i = 0;
			printf("----%s-%d\n", msg.c_str(), i++);

		}

		ByteBuffer packet;
		packet << "hello";
		pSession->sendPacketKCP(packet);
	}
}

