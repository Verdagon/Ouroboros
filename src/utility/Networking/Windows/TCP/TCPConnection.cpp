/*
 * TCPConnection.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: Evan
 */

#include "Protocol.h"
#include "../../../Utility.h"

#include <string.h>
#include <TCHAR.H>

using namespace Networking;
using namespace WinsockTCP;

ConnectionThreadImpl::ConnectionThreadImpl(bool connectSynchronously) :
		ConnectionThread(connectSynchronously),
		running(true) {
	waiter = Waiter::create();
	interruptor = Threading::Interruptor::create();
	waiter->addWaitable(interruptor);
}

ConnectionThreadImpl::~ConnectionThreadImpl() {
	
	trackedDelete(interruptor);
	
	trackedDelete(waiter);
	
}

void ConnectionThreadImpl::connect() {
	socket = connectInner();

	assert(socket);

	afterConnected();
}

void ConnectionThreadImpl::disconnect() {
	trackedDelete(socket);
	socket = NULL;

	afterDisconnected();
}

void *ConnectionThreadImpl::mainInner() {
	waiter->addWaitable(socket);

	while (running) {
		std::pair<Waitable *, int> result = waiter->waitUntil();
		if (result.first == interruptor) {
			while (running) {
				Event *event = getWaitingEvent(false);
				if (event == NULL)
					break;
				if (!handleEvent(event))
					assert(false);
				trackedDelete(event);
			}
		}
		else if (result.first == socket) {
			if (result.second & FD_READ) {
				Message header(0, 0);
				socket->peek(&header, sizeof(Message));

				assert(header.size >= sizeof(Message));
				assert(header.size < 10000); // curious

				Message *message = (Message *)trackedMalloc(header.size);
				socket->read(message, header.size);

				assert(header.size == message->size);
				assert(header.type == message->type);

				if (!handleMessage(message))
					assert(false);
				trackedFree(message);
			}
			else if (result.second & FD_CLOSE) {
				afterDisconnected();
			}
			else
				assert(false);
		}
		else
			assert(false);
	}

	return NULL;
}

bool ConnectionThreadImpl::handleEvent(Event *event_) {
	if (ConnectionThread::handleEvent(event_)) {
		return true;
	}
	else if (event_->is<ShutdownEvent>()) {
		afterDisconnected();
		running = false;
		return true;
	}
	else {
		return false;
	}
}

void ConnectionThreadImpl::onEventsAdded() {
	interruptor->interrupt();
}

void ConnectionThreadImpl::sendMessages(const std::list<Networking::Message *> &messages) {
	for (std::list<Message *>::const_iterator i = messages.begin(), iEnd = messages.end(); i != iEnd; i++) {
		Message *nextOutgoingMessage = *i;
		socket->send(nextOutgoingMessage, nextOutgoingMessage->size);
		trackedFree(nextOutgoingMessage);
	}
}
