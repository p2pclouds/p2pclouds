#include "connect_packet.h"

namespace P2pClouds {

	#define CMD_P2PCLOUDS_CONNECT_PACKET "p2pclouds_connect_package hello"
	#define CMD_P2PCLOUDS_CONNECT_ACK_PACKET "p2pclouds_connect_back_package ack:"
	#define CMD_P2PCLOUDS_DISCONNECT_PACKET "p2pclouds_disconnect_package request"

	bool ConnectPacket::isConnectPacket(ByteBuffer& datas)
	{
		return (datas.length() == sizeof(CMD_P2PCLOUDS_CONNECT_PACKET) &&
			memcmp(datas.data(), CMD_P2PCLOUDS_CONNECT_PACKET, sizeof(CMD_P2PCLOUDS_CONNECT_PACKET) - 1) == 0);
	}

	ByteBuffer ConnectPacket::makeConnectPacket(void)
	{
		ByteBuffer packet;
		packet << CMD_P2PCLOUDS_CONNECT_PACKET;
		return packet;
	}

	bool ConnectPacket::is_connect_ack_packet(ByteBuffer& datas)
	{
		return (datas.length() > sizeof(CMD_P2PCLOUDS_CONNECT_ACK_PACKET) &&
			memcmp(datas.data(), CMD_P2PCLOUDS_CONNECT_ACK_PACKET, sizeof(CMD_P2PCLOUDS_CONNECT_ACK_PACKET) - 1) == 0);
	}

	ByteBuffer ConnectPacket::make_connect_ack_packet(SessionID sessionID)
	{
		ByteBuffer packet;
		packet << CMD_P2PCLOUDS_CONNECT_ACK_PACKET;
		packet << sessionID;
		return packet;
	}

	SessionID ConnectPacket::get_sessionid_from_connect_ack_packet(ByteBuffer& datas)
	{
		datas.read_skip<char*>();
		SessionID sessionID;
		datas >> sessionID;
		return sessionID;
	}

	bool ConnectPacket::is_disconnect_packet(ByteBuffer& datas)
	{
		return (datas.length() > sizeof(CMD_P2PCLOUDS_DISCONNECT_PACKET) &&
			memcmp(datas.data(), CMD_P2PCLOUDS_DISCONNECT_PACKET, sizeof(CMD_P2PCLOUDS_DISCONNECT_PACKET) - 1) == 0);
	}

	ByteBuffer ConnectPacket::make_disconnect_packet(SessionID sessionID)
	{
		ByteBuffer packet;
		packet << CMD_P2PCLOUDS_DISCONNECT_PACKET;
		packet << sessionID;
		return packet;
	}

	SessionID ConnectPacket::get_sessionid_from_disconnect_packet(ByteBuffer& datas)
	{
		datas.read_skip<char*>();
		SessionID sessionID;
		datas >> sessionID;
		return sessionID;
	}
}
