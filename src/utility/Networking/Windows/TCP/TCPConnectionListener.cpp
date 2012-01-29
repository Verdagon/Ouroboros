/*
 * ConnectionListener.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: Evan
 */

#include "../../../TrackedAlloc.h"
#include "Protocol.h"

#include <string.h>
#include <TCHAR.H>

#ifdef _WIN32
	#define socklen_t int
#endif

using namespace Networking;
using namespace WinsockTCP;

ConnectionListenerThreadImpl::ConnectionListenerThreadImpl(const std::list<int> &portOptions_) :
		portOptions(portOptions_),
		listeningPort(0) {
	waiter = Waiter::create();
	interruptor = Threading::Interruptor::create();
	waiter->addWaitable(interruptor);
}

ConnectionListenerThreadImpl::~ConnectionListenerThreadImpl() {
	trackedDelete(interruptor);
	trackedDelete(waiter);
}

int ConnectionListenerThreadImpl::getListeningPort() {
	assert(listeningPort); // if its 0, it hasnt been set yet
	return listeningPort;
}

void *ConnectionListenerThreadImpl::main() {
	Socket *socket = Socket::openAndBind(portOptions);
	listeningPort = socket->getListeningPort();
	assert(listeningPort);
	waiter->addWaitable(socket);

	for (bool running = true; running; ) {

		socket->enterListenMode();

		std::pair<Waitable *, int> result = waiter->waitUntil();

		if (result.first == interruptor) {
			Event *event_ = getWaitingEvent();
			if (Thread::handleEvent(event_)) {

			}
			else if (event_->is<ShutdownEvent>()) {
				running = false;
			}
			else
				assert(false);
			trackedDelete(event_);
		}
		else if (result.first == socket) {
			if (result.second == FD_ACCEPT) {
				Socket *newSocket = socket->acceptFromMe();
				ConnectionThread *newThread = trackedNew(ConnectionThreadImplWithCustomSocket(newSocket));

				notify(new DCCallback1<ConnectionListenerThreadObserver, ConnectionThread *, &ConnectionListenerThreadObserver::afterConnectionEstablished>(newThread));
			}
			else if (result.second == FD_CLOSE) {
				running = false;
			}
			else
				assert(false);
		}
		else
			assert(false);
	}

	trackedDelete(socket);

	return NULL;
}

void ConnectionListenerThreadImpl::onEventsAdded() {
	interruptor->interrupt();
}
