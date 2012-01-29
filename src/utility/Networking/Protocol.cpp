/*
 * ConnectionThread.cpp
 *
 *  Created on: Dec 16, 2011
 *      Author: Evan
 */

#include "../TrackedAlloc.h"
#include "Protocol.h"

using namespace Networking;

struct DisconnectMessage : public Message {
	enum { TYPE = 0x1 };
	DisconnectMessage() : Message(sizeof(DisconnectMessage), TYPE) { }
};

class SendMessagesEvent : public Threading::Thread::Event {
public:
	std::list<Message *> messages;
	SendMessagesEvent(const std::list<Message *> &messages_) : messages(messages_) { }
};

class DisconnectEvent : public Threading::Thread::Event { };

// returns whether or not the event has been fully handled.
bool ConnectionThread::handleEvent(Event *event_) {
	if (Thread::handleEvent(event_)) {
		return true;
	}
	else if (SendMessagesEvent *event = event_->as<SendMessagesEvent>()) {
		sendMessages(event->messages);
		return true;
	}
	else if (event_->is<DisconnectEvent>()) {
		assert(connected);

		std::list<Message *> disconnectMessageList;
		disconnectMessageList.push_back(trackedMallocType(DisconnectMessage, DisconnectMessage()));
		sendMessages(disconnectMessageList);

		connected = false;
		notify(new DCCallback1<ConnectionThreadObserver, ConnectionThread *, &ConnectionThreadObserver::onDisconnected>(this));
		return true;
	}
	else
		return false;
}

bool ConnectionThread::handleMessage(Message *message) {
	assert(connected); // curious, how would we get a message if we're not connected?

	assert(isTrackedMalloc(message));

	if (message->type == DisconnectMessage::TYPE) {
		connected = false;
		notify(new DCCallback1<ConnectionThreadObserver, ConnectionThread *, &ConnectionThreadObserver::onDisconnected>(this));
		assert(isTrackedMalloc(message));
		return true;
	}
	else {
		sanityCheck();

		notify(new DCCallback2<ConnectionThreadObserver, ConnectionThread *, Message *, &ConnectionThreadObserver::onMessageReceived>(this, message));

		return true;
	}

	assert(isTrackedMalloc(message));
}

void ConnectionThread::afterDisconnected() {
	connected = false;
	notify(new DCCallback1<ConnectionThreadObserver, ConnectionThread *, &ConnectionThreadObserver::onDisconnected>(this));
}

void ConnectionThread::afterConnected() {
	connected = true;
	notify(new DCCallback1<ConnectionThreadObserver, ConnectionThread *, &ConnectionThreadObserver::onConnected>(this));
}

void ConnectionThread::raiseSendMessagesEvent(const std::list<Message *> &messages) {
	for (std::list<Message *>::const_iterator i = messages.begin(), iEnd = messages.end(); i != iEnd; i++)
		assert(isTrackedMalloc(*i));
	assert(connected);
	this->addEvent(trackedNew(SendMessagesEvent(messages)));
}

void ConnectionThread::raiseDisconnectEvent() {
	//	assert(connected); // we dont do this assert because it whines when we try to disconnect before it had a chance to connect.
	this->addEvent(trackedNew(DisconnectEvent()));
}

void *ConnectionThread::afterJumpIntoNewThread() {
	Thread::afterJumpIntoNewThread();

	if (connectSynchronously) {
		connect();
		if (connected)
			notify(new DCCallback1<ConnectionThreadObserver, ConnectionThread *, &ConnectionThreadObserver::onConnected>(this));
		else
			notify(new DCCallback1<ConnectionThreadObserver, ConnectionThread *, &ConnectionThreadObserver::onDisconnected>(this));
	}

	return afterJumpIntoNewThreadInner();
}

void ConnectionThread::beforeEndingThread() {
	if (connectSynchronously) {
		disconnect();
		connected = false;
		notify(new DCCallback1<ConnectionThreadObserver, ConnectionThread *, &ConnectionThreadObserver::onDisconnected>(this));
	}

	assert(!connected);

	Thread::beforeEndingThread();
}

void *ConnectionThread::main() {
	assert(connectSynchronously == connected);

	if (!connectSynchronously) {
		connect();
		if (connected)
			notify(new DCCallback1<ConnectionThreadObserver, ConnectionThread *, &ConnectionThreadObserver::onConnected>(this));
		else
			notify(new DCCallback1<ConnectionThreadObserver, ConnectionThread *, &ConnectionThreadObserver::onDisconnected>(this));
	}

	void *result = mainInner();

	if (!connectSynchronously) {
		disconnect();

		// the subclass's disconnect() needs to call disconnected() to set connected to false
		assert(!connected);
	}

	assert(!connected);

	return result;
}
