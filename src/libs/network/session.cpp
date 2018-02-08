#include "session.h"
#include "network_interface.h"
#include "connect_packet.h"

#include "log/log.h"

namespace P2pClouds {

	Session::Session(SessionID id, NetworkInterface& networkInterface, const asio::ip::udp::endpoint& remoteEndpoint)
	    : networkInterface_(networkInterface)
		, remoteEndpoint_(std::move(remoteEndpoint))
		, id_(id)
		, pKCP_(NULL)
		, lastRecvTime_(getTimeStamp())
	{
	}

	Session::~Session()
	{
		finalise();
	}

	bool Session::initialize()
	{
		return init_kcp();
	}

	bool Session::finalise()
	{
		LOG_INFO("send disconnect packet! {}", c_str());
		ByteBuffer packet = ConnectPacket::make_disconnect_packet(id());
		sendPacket(packet);
		return fina_kcp();
	}

	std::string Session::c_str()
	{
		return fmt::format("id={}, endpoint={}:{}", id(), endpoint().address().to_string(), endpoint().port());
	}

	std::shared_ptr<Session> Session::create(NetworkInterface& networkInterface,
		uint32 sessionID, const asio::ip::udp::endpoint& remoteEndpoint)
	{
		std::shared_ptr<Session> ptr = std::make_shared<Session>(sessionID, networkInterface, remoteEndpoint);

		if (ptr)
		{
			LOG_INFO("new session from: {}", ptr->c_str());

			if (!ptr->initialize())
				return NULL;
		}

		return ptr;
	}

	SessionID Session::createNewSessionID(SessionID arrayIndex)
	{
		assert((std::is_same<SessionID, uint32>::value));

		SessionID sessionID = 0;

		SessionID index = arrayIndex;
		index <<= 16;

		sessionID |= index;

		std::random_device rd;
		std::default_random_engine e(rd());
		std::uniform_int_distribution<> u(0, 0xffff);
		SessionID rnd = u(e);

		sessionID |= rnd;
		return sessionID;
	}

	bool Session::init_kcp()
	{
		pKCP_ = ikcp_create(id(), (void*)this);
		pKCP_->output = &Session::output;

		// normal
		//ikcp_nodelay(pKCP_, 0, 40, 0, 0);

		// fast speed
		ikcp_nodelay(pKCP_, 1, 10, 2, 1);
		return true;
	}

	bool Session::fina_kcp()
	{
		ikcp_release(pKCP_);
		pKCP_ = NULL;
		return true;
	}

	bool Session::update(time_t timeStamp)
	{
		ikcp_update(pKCP_, (IUINT32)(timeStamp & 0xfffffffful));

		if (isTimeout())
		{
			LOG_INFO("session timeout: {}", c_str());
			handTimeout();
			return false;
		}

		return true;
	}

	int Session::output(const char *buf, int len, ikcpcb *kcp, void *user)
	{
		((Session*)user)->sendPacket(buf, len);
		return 0;
	}

	size_t Session::sendPacket(ByteBuffer& datas)
	{
		return networkInterface_.sendPacket(datas, endpoint());
	}

	size_t Session::sendPacket(const char *buf, int len)
	{
		return networkInterface_.sendPacket(buf, len, endpoint());
	}

	void Session::sendPacketKCP(ByteBuffer& datas)
	{
		int sentSize = ikcp_send(pKCP_, (const char*)datas.data(), datas.length());
		if (sentSize < 0)
		{
			LOG_ERROR("send_kcp_msg(): sentSize < 0! {}", c_str());
		}
	}

	void Session::input(ByteBuffer& datas, const asio::ip::udp::endpoint& remoteEndpoint)
	{
		lastRecvTime_ = getTimeStamp();
		remoteEndpoint_ = remoteEndpoint;

		ikcp_input(pKCP_, (const char*)datas.data(), datas.length());
		datas.wpos(0);

		{
			int bytes_recvd = ikcp_recv(pKCP_, (char*)datas.data(), datas.size());
			if (bytes_recvd <= 0)
			{
				LOG_INFO("Session::input(): recvd_bytes <= 0! {}", c_str());
			}
			else
			{
				assert(bytes_recvd < datas.size());
				datas.wpos(bytes_recvd);
				networkInterface_.callEventCallbackFunc(shared_from_this(), NetEventType::NetRcvMsg, &datas);
			}
		}
	}

	bool Session::isTimeout(time_t timeStamp) const
	{
		if (lastRecvTime_ == 0)
			return false;
		
		return (timeStamp == 0 ? getTimeStamp() : timeStamp) - lastRecvTime_ > P2PCLOUDS_CONNECTION_TIMEOUT_TIME;
	}

	void Session::handTimeout(void)
	{
		networkInterface_.callEventCallbackFunc(shared_from_this(), NetEventType::NetDisconnect, NULL);
	}
}
