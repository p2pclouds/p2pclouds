#include "network_interface.h"
#include "connect_packet.h"
#include "session.h"

#include "log/log.h"

namespace P2pClouds {

	NetworkInterface::NetworkInterface(asio::io_service& io_service, const std::string& address, int udp_port)
	    : udp_socket_(io_service, asio::ip::udp::endpoint(asio::ip::address::from_string(address), udp_port))
		, stopped_(true)
		, remoteEndpoint_()
		, buffer_(1024 * 32)
		, tick_timer_(udp_socket_.get_io_service())
		, sessions_()
		, event_callback_()
	{
		buffer_.data_resize(1024 * 32);
	}

	NetworkInterface::~NetworkInterface()
	{
		stopAll();
	}

	bool NetworkInterface::initialize()
	{
		stopped_ = false;
		hookAsyncReceive();
		hookUpdateTimer();
		
		return true;
	}

	bool NetworkInterface::finalise()
	{
		stopAll();
		return true;
	}

	void NetworkInterface::stopAll()
	{
		stopped_ = true;

        if (udp_socket_.is_open())
        {
            udp_socket_.cancel();
            udp_socket_.close();
        }
        
		sessions_.clear();
	}

	bool NetworkInterface::connect(const std::string& address, int udp_port)
	{
		asio::ip::udp::endpoint remoteEndpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(address), udp_port);

		// send a connect cmd.
		ByteBuffer packet = ConnectPacket::makeConnectPacket();
		LOG_INFO("send connect packet! endpoint={}:{}", remoteEndpoint.address().to_string(), remoteEndpoint.port());
		sendPacket(packet, remoteEndpoint);
		return true;
	}

	void NetworkInterface::disconnect(SessionID sessionID)
	{
		std::shared_ptr<Session> session = findSession(sessionID);
		if (!session)
			return;

		LOG_INFO("session disconnect(): {}", session->c_str());
		callEventCallbackFunc(session, NetEventType::NetDisconnect, NULL);
		removeSession(sessionID);
	}

	void NetworkInterface::callEventCallbackFunc(std::shared_ptr<Session> pSession, NetEventType event_type, ByteBuffer* pdatas)
	{
		event_callback_(pSession, event_type, pdatas);
	}

	void NetworkInterface::setEventCallback(const std::function<net_event_callback_t>& eventCallback)
	{
		event_callback_ = eventCallback;
	}

	void NetworkInterface::hookUpdateTimer(void)
	{
		if (stopped_)
			return;

		tick_timer_.expires_from_now(std::chrono::milliseconds(5));
		tick_timer_.async_wait(std::bind(&NetworkInterface::handleUpdateTimer, this));
	}

	void NetworkInterface::handleUpdateTimer(void)
	{
		hookUpdateTimer();
		
		time_t timeStamp = getTimeStamp();

		for (auto iter = sessions_.begin(); iter != sessions_.end();)
		{
			if (!iter->second->update(timeStamp))
				sessions_.erase(iter++);
			else
				iter++;
		}
	}

	void NetworkInterface::hookAsyncReceive(void)
	{
		if (stopped_)
			return;

		udp_socket_.async_receive_from(
			asio::buffer(buffer_.data(), buffer_.size()), remoteEndpoint_,

			std::bind(&NetworkInterface::handleReceiveFrom, this,
				std::placeholders::_1,
				std::placeholders::_2)
		);
	}

	void NetworkInterface::handleReceiveFrom(const std::error_code& error, size_t bytes_recvd)
	{
		if (!error && bytes_recvd > 0)
		{
#if ENABLE_UDP_PACKET_LOG
			LOG_DEBUG("udpRecv(): senderaddr={}:{}, size={}", remoteEndpoint_.address().to_string(), remoteEndpoint_.port(), bytes_recvd);
#endif

			assert(bytes_recvd < buffer_.size());
			buffer_.rpos(0);
			buffer_.wpos(bytes_recvd);

			if (ConnectPacket::isConnectPacket(buffer_))
			{
				handleConnectPacket();
				goto END;
			}
			else if (ConnectPacket::is_connect_ack_packet(buffer_))
			{
				handleConnectAckPacket();
				goto END;
			}
			else if (ConnectPacket::is_disconnect_packet(buffer_))
			{
				handleDisconnectPacket();
				goto END;
			}
			else
			{
				handlePacketKCP(bytes_recvd);
			}
		}
		else
		{
			LOG_ERROR("handleReceiveFrom error end! error: {}, bytes_recvd: {}\n", error.message().c_str(), bytes_recvd);
		}

	END:
		hookAsyncReceive();
	}

	void NetworkInterface::handlePacketKCP(size_t bytes_recvd)
	{
		SessionID sessionID = ikcp_getconv((const char*)buffer_.data());

		std::shared_ptr<Session> session = findSession(sessionID);
		if (!session)
		{
			LOG_ERROR("handlePacketKCP(): connection not exist with sessionID: {}", sessionID);
			return;
		}

		if(session->endpoint() != remoteEndpoint_)
		{
			LOG_ERROR("handlePacketKCP(): endpoint({}:{}) != sessionEndPoint({}:{}) with sessionID: {}", remoteEndpoint_.address().to_string(), remoteEndpoint_.port(),
				session->endpoint().address().to_string(), session->endpoint().port(), sessionID);

			return;
		}

		session->input(buffer_, remoteEndpoint_);
	}

	size_t NetworkInterface::sendPacket(const char *buf, int len, const asio::ip::udp::endpoint& endpoint)
	{
#if ENABLE_UDP_PACKET_LOG
		LOG_DEBUG("udpSend(): senderAddr={}:{}, size={}", endpoint.address().to_string(), endpoint.port(), len);
#endif

		return udp_socket_.send_to(asio::buffer(buf, len), endpoint);
	}

	void NetworkInterface::handleConnectPacket()
	{
		static SessionID sessionID = 1;
		std::shared_ptr<Session> session = Session::create(*this, sessionID++, remoteEndpoint_);

		ByteBuffer packet = ConnectPacket::make_connect_ack_packet(session->id());

		udp_socket_.send_to(asio::buffer(packet.data(), packet.wpos()), remoteEndpoint_);
		
		addSession(session->id(), session);
		callEventCallbackFunc(session, NetEventType::NetConnect, NULL);
	}

	void NetworkInterface::handleConnectAckPacket()
	{
		LOG_INFO("connect {}:{} success!", remoteEndpoint_.address().to_string(), remoteEndpoint_.port());
		SessionID sessionID = ConnectPacket::get_sessionid_from_connect_ack_packet(buffer_);
		std::shared_ptr<Session> session = Session::create(*this, sessionID, remoteEndpoint_);
		addSession(session->id(), session);
		callEventCallbackFunc(session, NetEventType::NetConnect, NULL);
	}

	void NetworkInterface::handleDisconnectPacket()
	{
		LOG_INFO("session disconnected: {}:{} !", remoteEndpoint_.address().to_string(), remoteEndpoint_.port());
		SessionID sessionID = ConnectPacket::get_sessionid_from_connect_ack_packet(buffer_);

		std::shared_ptr<Session> session = findSession(sessionID);

		if (session)
		{
			callEventCallbackFunc(session, NetEventType::NetDisconnect, NULL);
			removeSession(sessionID);
		}
	}
	
	bool NetworkInterface::addSession(SessionID sessionID, std::shared_ptr<Session> session)
	{
		auto iter = sessions_.find(sessionID);

		if (iter != sessions_.end())
			return false;

		sessions_[sessionID] = session;
		return true;
	}

	bool NetworkInterface::removeSession(SessionID sessionID)
	{
		sessions_.erase(sessionID);
		return true;
	}

	std::shared_ptr<Session> NetworkInterface::findSession(SessionID sessionID)
	{
		auto iter = sessions_.find(sessionID);

		if (iter == sessions_.end())
			return NULL;

		return iter->second;
	}
}
