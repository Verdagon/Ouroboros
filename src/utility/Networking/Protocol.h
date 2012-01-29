/*
 * Protocol.h
 *
 *  Created on: Dec 16, 2011
 *      Author: Evan
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "../Threading/Thread.h"
#include "../TrackedAlloc.h"
#include "../Observing.h"

namespace Networking {

	namespace Messages {

		struct Message {
			uint16_t size;
			uint16_t type;
			Message(uint16_t size_, uint16_t type_) : size(size_), type(type_) {
				// Messages should only ever be locals or trackedMalloc's
//				assert(!isTrackedNew(this));
			}

			static Message *clone(const Message *source) {
				Message *result = (Message *)trackedMalloc(source->size);
				memcpy(result, source, source->size);
				return result;
			}
		};

	}

	using Messages::Message;

	class ConnectionThread;

	class ConnectionThreadObserver {
	public:
		ConnectionThreadObserver() {
			FLAG();
		}
		~ConnectionThreadObserver() {
			std::cout << "Deleting obs " << this << std::endl;
			FLAG();
		}
		virtual void onMessageReceived(ConnectionThread *connection, Networking::Message *message) { }
		virtual void onConnected(ConnectionThread *connection) { }
		virtual void onConnectFailed(ConnectionThread *connection) { }
		virtual void onDisconnected(ConnectionThread *connection) { }
	};

	class ConnectionThread : public Threading::Thread, public Observed<ConnectionThreadObserver> {
		bool connectSynchronously;
		bool connected;

	protected:
		virtual bool handleEvent(Event *event_);
		virtual bool handleMessage(Message *message);

		// subclass must call these
		void afterDisconnected();
		void afterConnected();

		virtual void *afterJumpIntoNewThread();
		virtual void beforeEndingThread();
		virtual void *main();

		virtual void sendMessages(const std::list<Message *> &outgoingMessages) = 0;
		virtual void connect() = 0;
		virtual void disconnect() = 0;
		virtual void *afterJumpIntoNewThreadInner() = 0;
		virtual void *beforeEndingThreadInner() = 0;
		virtual void *mainInner() = 0;

	public:
		ConnectionThread(bool connectSynchronously_) :
			Thread("Conn"), connectSynchronously(connectSynchronously_), connected(false) { }

		void raiseSendMessagesEvent(Message *message) {
			std::list<Message *> messages;
			messages.push_back(message);
			raiseSendMessagesEvent(messages);
		}
		void raiseSendMessagesEvent(const std::list<Message *> &message);
		void raiseDisconnectEvent();

		bool isConnected() const { return connected; }
	};




	class ConnectionListenerThreadObserver {
	public:
		virtual void afterConnectionEstablished(ConnectionThread *newConnection) = 0;
	};

	class ConnectionListenerThread : public Threading::Thread, public Observed<ConnectionListenerThreadObserver> {
	public:
		virtual int getListeningPort() = 0;

		ConnectionListenerThread() :
			Thread("Listen") { }
	};





	// A protocol also depends on the platform.
	// Any given protocol class is a certain network protocol on a certain platform.
	// You guessed it, the Protocol is simply a factory for connection threads and connection listener threads.
	class Protocol {
	public:
		enum Type { TCP };
		static Protocol *create(Type type);

		virtual ConnectionListenerThread *createConnectionListenerThread(const std::list<int> &portOptions) = 0;

		virtual ConnectionThread *createConnectionTo(const char *hostName, const std::list<int> &portOptions, bool connectSynchronously) = 0;
	};
}

#endif /* PROTOCOL_H_ */
