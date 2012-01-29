/*
 * ThreadedConnection.cpp
 *
 *  Created on: Jan 8, 2012
 *      Author: Evan
 */

#include "ThreadedConnection.h"

using namespace Networking;

class ReceiveMessagesEvent : public Threading::Thread::Event {
public:
	std::list<Message *> messages;
};

ThreadedConnectionThread::ThreadedConnectionThread() :
		ConnectionThread(true),
		otherSide(NULL),
		waitForEvents(Waiter::create()),
		waitForEventsInterruptor(Threading::Interruptor::create()) {
	waitForEvents->addWaitable(waitForEventsInterruptor);
}

ThreadedConnectionThread::~ThreadedConnectionThread() {
	trackedDelete(waitForEventsInterruptor);
	trackedDelete(waitForEvents);
}

std::pair<ThreadedConnectionThread *, ThreadedConnectionThread *> ThreadedConnectionThread::create() {
	ThreadedConnectionThread *first = trackedNew(ThreadedConnectionThread());
	ThreadedConnectionThread *second = trackedNew(ThreadedConnectionThread());

	first->otherSide = second;
	second->otherSide = first;

	return std::pair<ThreadedConnectionThread *, ThreadedConnectionThread *>(first, second);
}

void *ThreadedConnectionThread::mainInner() {
	for (bool running = true; running; ) {
		waitForEvents->waitUntil();
		for (Event *event_; running && (event_ = getWaitingEvent(false)); trackedDelete(event_)) {
			if (handleEvent(event_)) { }
			else if (event_->is<ShutdownEvent>())
				running = false;
			else if (ReceiveMessagesEvent *event = event_->as<ReceiveMessagesEvent>()) {
				for (std::list<Message *>::iterator i = event->messages.begin(), iEnd = event->messages.end(); i != iEnd; i++) {
					Message *message = *i;
					assert(isTrackedMalloc(message));
					handleMessage(message);
					trackedFree(message);
				}
			}
		}
	}

	return NULL;
}

void ThreadedConnectionThread::sendMessages(const std::list<Message *> &outgoingMessages) {
	ReceiveMessagesEvent *event = trackedNew(ReceiveMessagesEvent);
	for (std::list<Message *>::const_iterator i = outgoingMessages.begin(), iEnd = outgoingMessages.end(); i != iEnd; i++) {
		assert(isTrackedMalloc(*i));
		event->messages.push_back(*i);
	}

	otherSide->addEvent(event);
}

void ThreadedConnectionThread::onEventsAdded() {
	waitForEventsInterruptor->interrupt();
}
