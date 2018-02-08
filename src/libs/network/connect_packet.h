#pragma once

#include "common.h"

namespace P2pClouds {

	class ConnectPacket
	{
	public:
		static bool isConnectPacket(ByteBuffer& datas);
		static ByteBuffer makeConnectPacket(void);

		static ByteBuffer make_connect_ack_packet(SessionID sessionID);
		static bool is_connect_ack_packet(ByteBuffer& datas);
		static SessionID get_sessionid_from_connect_ack_packet(ByteBuffer& datas);

		static ByteBuffer make_disconnect_packet(SessionID sessionID);
		static bool is_disconnect_packet(ByteBuffer& datas);
		static SessionID get_sessionid_from_disconnect_packet(ByteBuffer& datas);
	};

}
