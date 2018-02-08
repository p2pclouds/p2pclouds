#pragma once

#include "common.h"

namespace P2pClouds {

	class Session;

	class NetworkInterface
	{
	public:
		NetworkInterface(asio::io_service& io_service, const std::string& address, int udp_port = LISTEN_PORT);
		virtual ~NetworkInterface();

		bool initialize();
		bool finalise();

		void stopAll();

		bool connect(const std::string& address, int udp_port = LISTEN_PORT);
		void disconnect(SessionID sessionID);

		// user level send packet.
		size_t sendPacket(ByteBuffer& datas, const asio::ip::udp::endpoint& endpoint) {
			return sendPacket((const char *)datas.data(), (int)datas.length(), endpoint);
		}

		size_t sendPacket(const char *buf, int len, const asio::ip::udp::endpoint& endpoint);

		void callEventCallbackFunc(std::shared_ptr<Session> pSession, NetEventType event_type, ByteBuffer* pdatas);
		void setEventCallback(const std::function<net_event_callback_t>& eventCallback);

	protected:
		void hookAsyncReceive(void);
		void handleReceiveFrom(const std::error_code& error, size_t bytes_recvd);
		void handlePacketKCP(size_t bytes_recvd);

		void handleConnectPacket();
		void handleConnectAckPacket();
		void handleDisconnectPacket();

		void hookUpdateTimer(void);
		void handleUpdateTimer(void);

		bool addSession(SessionID sessionID, std::shared_ptr<Session> session);
		bool removeSession(SessionID sessionID);
		std::shared_ptr<Session> findSession(SessionID sessionID);

	protected:
		asio::ip::udp::socket udp_socket_;
		bool stopped_;

		asio::ip::udp::endpoint remoteEndpoint_;

		//enum { UDP_PACKET_MAX_LENGTH = 548 }; // maybe 1472 will be ok.
		enum { UDP_PACKET_MAX_LENGTH = 1080 }; // (576-8-20 - 8) * 2

		ByteBuffer buffer_;

		asio::steady_timer tick_timer_;

		std::map< SessionID, std::shared_ptr<Session> > sessions_;

		std::function<net_event_callback_t> event_callback_;
	};

}
