/*
 * WinsockTCPProtocol.h
 *
 *  Created on: Dec 16, 2011
 *      Author: Evan
 */

#ifndef WINSOCKTCPPROTOCOL_H_
#define WINSOCKTCPPROTOCOL_H_

#include "../../../TrackedAlloc.h"
#include "../../../Utility.h"
#include "../../Protocol.h"
#include "../../../Wait/Windows/WindowsWait.h"
#include "../../../Threading/Interruptor.h"
#include <winsock2.h>

namespace Networking {

	namespace WinsockTCP {

		std::string GetLastErrorMessage(DWORD last_error, bool stripTrailingLineFeed = true);

		class ProtocolImpl;

		class Socket : public WindowsWaitable {
			SOCKET rawSocket;
			bool nonblockingModeEnabled;

			void recv(void *buffer, int bufferSizeBytes, bool peek);
			void setNonblockingMode(bool enabled);
			bool isNonblockingModeEnabled() const;

			Socket(SOCKET socket_);

		public:
			HANDLE beforeWaitStarted();
			void afterWaitFinished(HANDLE handle);
			int onFire(HANDLE handle);

			~Socket();

			void enterListenMode();
			void close();
			void read(void *buffer, int bufferSizeBytes);
			void peek(void *buffer, int bufferSizeBytes);
			void send(void *buffer, int bufferSizeBytes);
			Socket *acceptFromMe();

			static Socket *createConnectionTo(const char *hostName, const std::list<int> &portOptions);
			static Socket *openAndBind(const std::list<int> &portOptions);

			int getListeningPort();
		};

		class ConnectionThreadImpl : public ConnectionThread {
			Socket *socket;
			bool running;
			Waiter *waiter;
			Threading::Interruptor *interruptor;

			void handleSocketActivity();
			virtual bool handleEvent(Event *event_);
			virtual void onEventsAdded();
			virtual void sendMessages(const std::list<Networking::Message *> &messages);

			virtual void connect();
			virtual void disconnect();
			virtual void *afterJumpIntoNewThreadInner() { return NULL; }
			virtual void *beforeEndingThreadInner() { return NULL; }

		protected:
			virtual Socket *connectInner() = 0;

			virtual void *mainInner();

			ConnectionThreadImpl(bool connectSynchronously);
			~ConnectionThreadImpl();
		};

		class ConnectionThreadImplConnectingToRemote : public ConnectionThreadImpl {
			const std::string hostName;
			const std::list<int> portOptions;

		public:
			ConnectionThreadImplConnectingToRemote(const std::string &hostName_, const std::list<int> &portOptions_, bool connectSynchronously) :
				ConnectionThreadImpl(connectSynchronously),
				hostName(hostName_),
				portOptions(portOptions_) { }

			Socket *connectInner() {
				return Socket::createConnectionTo(hostName.c_str(), portOptions);
			}
		};

		class ConnectionThreadImplWithCustomSocket : public ConnectionThreadImpl {
			Socket *socket;

		public:
			ConnectionThreadImplWithCustomSocket(Socket *socket_) :
					ConnectionThreadImpl(true),
					socket(socket_) {
				assert(isTrackedNew(socket));
			}

			Socket *connectInner() {
				Socket *result = socket;
				socket = NULL;
				return result;
			}

			~ConnectionThreadImplWithCustomSocket() {
				if (socket)
					trackedDelete(socket);
			}
		};


		class ConnectionListenerThreadImpl : public ConnectionListenerThread {
			ProtocolImpl *protocol;
			Waiter *waiter;
			Threading::Interruptor *interruptor;
			const std::list<int> portOptions;
			int listeningPort;

		public:
			ConnectionListenerThreadImpl(const std::list<int> &portOptions);
			~ConnectionListenerThreadImpl();
			void *main();
			virtual void onEventsAdded();
			virtual int getListeningPort();
		};


		class ProtocolImpl : public Networking::Protocol {
			static bool winsockInitialized;

		public:
			ProtocolImpl() {
				if (!winsockInitialized) {
					WSAData wsaData;
					if (int nCode = WSAStartup(MAKEWORD(2, 2), &wsaData)) {
						std::cerr << "WSAStartup() returned error code " << nCode << "." << std::endl;
					}
					winsockInitialized = true;
				}
			}

			virtual ConnectionListenerThread *createConnectionListenerThread(const std::list<int> &portOptions) {
				return trackedNew(ConnectionListenerThreadImpl(portOptions));
			}

			virtual ConnectionThread *createConnectionTo(const char *hostName, const std::list<int> &portOptions, bool connectSynchronously) {
				return trackedNew(ConnectionThreadImplConnectingToRemote(hostName, portOptions, connectSynchronously));
			}
		};

	}

}

#endif /* WINSOCKTCPPROTOCOL_H_ */
