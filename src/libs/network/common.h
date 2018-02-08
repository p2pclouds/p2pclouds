#pragma once

#include "common/common.h"
#include "common/byte_buffer.h"

#include <ikcp.h>

namespace P2pClouds {

	#define P2PCLOUDS_CONNECTION_TIMEOUT_TIME 10 * 1000 // default is 10 seconds.

	#define LISTEN_PORT 27776

	typedef uint32 SessionID;

    enum NetEventType
    {
        NetConnect,
		NetDisconnect,
		NetRcvMsg,
		NetLagNotify,
		NetTimeout,

        NetCountOfEventType
    };

    const char* netEventType2Str(NetEventType eventType);

	class Session;
    typedef void(net_event_callback_t)(std::shared_ptr<Session> /*Session*/, NetEventType /*event_type*/, ByteBuffer* /*datas*/);


}
