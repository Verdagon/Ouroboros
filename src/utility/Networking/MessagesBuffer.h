/*
 * MessagesBuffer.h
 *
 *  Created on: Jan 8, 2012
 *      Author: Evan
 */

#ifndef MESSAGESBUFFER_H_
#define MESSAGESBUFFER_H_

#include "../Threading/Thread.h"
#include "Protocol.h"
#include <queue>

namespace Networking {

//	class IThreadProxy : public Threading::IThread {
//	protected:
//		virtual Threading::IThread *getUnderlyingThread() = 0;
//
//	public:
//		virtual void *jumpIntoNewThreadAndStart() { return getUnderlyingThread()->jumpIntoNewThreadAndStart(); }
//		virtual bool running() const { return getUnderlyingThread()->running(); }
//		virtual std::string getThreadName() const { return getUnderlyingThread()->getThreadName(); }
//		virtual bool join(double timeout = -1) { return getUnderlyingThread()->join(timeout); }
//		virtual bool shutdownAndJoin(double timeout = -1) { return getUnderlyingThread()->shutdownAndJoin(timeout); }
//		virtual void *getReturnValue() { return getUnderlyingThread()->getReturnValue(); }
//	};
//
//
//
//	class IConnectionThreadProxy : public IConnectionThread, public IThreadProxy {
//	protected:
//		virtual IConnectionThread *getUnderlyingConnectionThread() = 0;
//		virtual Threading::IThread *getUnderlyingThread() { return getUnderlyingConnectionThread(); }
//
//	public:
//		virtual void raiseSendMessagesEvent(const std::list<Message *> &messages) { getUnderlyingConnectionThread()->raiseSendMessagesEvent(messages); }
//		virtual void raiseDisconnectEvent() { getUnderlyingConnectionThread()->raiseDisconnectEvent(); }
//		virtual bool isConnected() const { return getUnderlyingConnectionThread()->isConnected(); }
//	};

	class MessageSender {
		ConnectionThread *connection;

	public:
		MessageSender(ConnectionThread *connection_) : connection(connection_) { }

		void addMessage(Message *message) {
			connection->raiseSendMessagesEvent(message);
		}
	};

	class BatchedMessageSender {
		ConnectionThread *connection;

		Threading::Mutexed<std::list<Networking::Message *> > mutexedOutgoingMessages;

	public:
		BatchedMessageSender(ConnectionThread *connection_) : connection(connection_) { }

		void addMessage(Message *message) {
			std::list<Networking::Message *> &outgoingMessages = mutexedOutgoingMessages.lockAndGet();
			outgoingMessages.push_back(message);
			mutexedOutgoingMessages.unlock();
		}

		void flushAddedMessages() {
			std::list<Networking::Message *> &outgoingMessages = mutexedOutgoingMessages.lockAndGet();
			if (!outgoingMessages.empty()) {
				connection->raiseSendMessagesEvent(outgoingMessages);
				outgoingMessages.clear();
			}
			mutexedOutgoingMessages.unlock();
		}
	};

	class ReceivedMessagesBuffer : public ConnectionThreadObserver {
	private:
		Threading::Mutexed<std::queue<Networking::Message *> > mutexedMessages;

	public:
		~ReceivedMessagesBuffer() {
			std::queue<Networking::Message *> &messages = mutexedMessages.lockAndGet();

			if (!messages.empty()) {
				std::cout << "Warning, deleting received messages buffer when messages still in it" << std::endl;
				while (!messages.empty()) {
					trackedFree(messages.front());
					messages.pop();
				}
			}

			mutexedMessages.unlock();
		}

		void onMessageReceived(ConnectionThread *connection, Networking::Message *message) {
			std::cout << "buffer received message!" << std::endl;
			mutexedMessages.lockAndGet().push(Networking::Message::clone(message));
			mutexedMessages.unlock();
		}

		int numMessagesWaiting() {
			const std::queue<Networking::Message *> &messages = mutexedMessages.lockAndGet();
			int result = messages.size();
			mutexedMessages.unlock();
			return result;
		}

		Networking::Message *getNextMessage() {
			std::queue<Networking::Message *> &messages = mutexedMessages.lockAndGet();
			Networking::Message *message = NULL;
			if (!messages.empty()) {
				message = messages.front();
				messages.pop();
			}
			mutexedMessages.unlock();
			return message;
		}
	};

}

#endif /* MESSAGESBUFFER_H_ */
