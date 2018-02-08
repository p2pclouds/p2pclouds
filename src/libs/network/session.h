#pragma once

#include "common.h"



namespace P2pClouds {

	class NetworkInterface;

	class Session : public std::enable_shared_from_this<Session>
	{
	public:
		Session(SessionID id, NetworkInterface& networkInterface, const asio::ip::udp::endpoint& remoteEndpoint);
		virtual ~Session();

		static std::shared_ptr<Session> create(NetworkInterface& networkInterface,
			uint32 sessionID, const asio::ip::udp::endpoint& remoteEndpoint);

		// Creating ID by the index of an array
		static SessionID createNewSessionID(SessionID arrayIndex);

		bool initialize();
		bool finalise();

		SessionID id() const {
			return id_;
		}

		const asio::ip::udp::endpoint& endpoint() const {
			return remoteEndpoint_;
		}

		// user level send packet.
		void sendPacketKCP(ByteBuffer& datas);
		size_t sendPacket(ByteBuffer& datas);

		bool update(time_t timeStamp);

		// changing remoteEndpoint at every packet. Because we allow connection change ip or port. we using conv to indicate a connection.
		void input(ByteBuffer& datas, const asio::ip::udp::endpoint& remoteEndpoint);

		bool isTimeout(time_t timeStamp = 0) const;
		void handTimeout(void);

		std::string c_str();

	protected:
		bool init_kcp();
		bool fina_kcp();
		static int output(const char *buf, int len, ikcpcb *kcp, void *user);
		size_t sendPacket(const char *buf, int len);

	protected:
		NetworkInterface& networkInterface_;
		asio::ip::udp::endpoint remoteEndpoint_;
		SessionID id_;
		ikcpcb* pKCP_;
		uint64 lastRecvTime_;
	};

}
