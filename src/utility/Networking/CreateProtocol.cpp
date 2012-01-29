/*
 * CreateProtocol.cpp
 *
 *  Created on: Dec 16, 2011
 *      Author: Evan
 */

#include "../TrackedAlloc.h"
#include "Protocol.h"

using namespace Networking;

#ifdef _WIN32

#include "Windows/TCP/Protocol.h"
Protocol *Protocol::create(Type type) { return trackedNew(WinsockTCP::ProtocolImpl()); }

#else
#error "Unknown platform, cant create networking protocol"
#endif
