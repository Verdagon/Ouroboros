/*
 * ThreadedConnection.h
 *
 *  Created on: Jan 8, 2012
 *      Author: Evan
 */

#ifndef THREADEDCONNECTION_H_
#define THREADEDCONNECTION_H_

#include "Protocol.h"
#include "../Wait/Wait.h"
#include "../Threading/Interruptor.h"

namespace Networking {

	class ThreadedConnectionThread : public ConnectionThread {
		ThreadedConnectionThread *otherSide;

		Waiter *waitForEvents;
		Threading::Interruptor *waitForEventsInterruptor;

		virtual void sendMessages(const std::list<Message *> &outgoingMessages);
		void *mainInner();
		void connect() { afterConnected(); }
		void disconnect() { afterDisconnected(); }
		void *afterJumpIntoNewThreadInner() { return NULL; }
		void *beforeEndingThreadInner() { return NULL; }

		ThreadedConnectionThread();
		void onEventsAdded();

	public:
		static std::pair<ThreadedConnectionThread *, ThreadedConnectionThread *> create();
		~ThreadedConnectionThread();
	};

}


#endif /* THREADEDCONNECTION_H_ */
