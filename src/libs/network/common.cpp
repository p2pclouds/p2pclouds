#include "common.h"

namespace P2pClouds {

    const char* netEventType2Str(NetEventType eventType)
    {
        switch (eventType)
        {
            case NetConnect: return "NetConnect";
            case NetDisconnect: return "NetDisconnect";
            case NetRcvMsg: return "NetRcvMsg";
            case NetLagNotify: return "NetLagNotify";
			case NetTimeout: return "NetTimeout";
            default: return "NetEventUnknown";
        }
    }
}
