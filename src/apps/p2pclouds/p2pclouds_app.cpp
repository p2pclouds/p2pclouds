#include "p2pclouds_app.h"
#include "network/session.h"
#include "network/network_interface.h"

#include "log/log.h"

namespace P2pClouds {

	P2pCloudsApp::P2pCloudsApp()
		: App()
	{
	}

	P2pCloudsApp::~P2pCloudsApp()
	{
	}

	bool P2pCloudsApp::initialize()
	{
		bool ret = App::initialize();
		return ret;
	}

	bool P2pCloudsApp::finalise()
	{
		return App::finalise();
	}

	bool P2pCloudsApp::run()
	{
		return App::run();
	}

	void P2pCloudsApp::netEventCallback(std::shared_ptr<Session> pSession, NetEventType event_type, ByteBuffer* pdatas)
	{
		LOG_TRACE("netEventCallback: sessionID:{} type: {}", pSession->id(), netEventType2Str(event_type));
		if (event_type == NetRcvMsg)
		{
			std::string msg;
			(*pdatas) >> msg;
			printf("----%s\n", msg.c_str());


		}

		ByteBuffer packet;
		packet << "hello";
		pSession->sendPacketKCP(packet);
	}
}
