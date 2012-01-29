
#include "../../../TrackedAlloc.h"
#include "Protocol.h"

using namespace Networking;
using namespace WinsockTCP;

HANDLE Socket::beforeWaitStarted() {
	setNonblockingMode(true);
	WSAEVENT socketEventHandle = WSACreateEvent();
	WSAEventSelect(rawSocket, socketEventHandle, FD_ACCEPT | FD_READ | FD_CLOSE);
	return socketEventHandle;
}

void Socket::afterWaitFinished(HANDLE handle) {
	// We need to close it here because having an event
	// open puts the socket in non-blocking mode. we need it to be in blocking mode for
	// things like accept().
	WSAEventSelect(rawSocket, NULL, 0);
	WSACloseEvent(handle);
}

int Socket::onFire(HANDLE handle) {
	WSANETWORKEVENTS networkEventsFired;
	int error = WSAEnumNetworkEvents(rawSocket, handle, &networkEventsFired);
	if (error) {
		std::cout << GetLastErrorMessage(GetLastError(), true) << std::endl;
		assert(false);
	}
	return networkEventsFired.lNetworkEvents;
}

void Socket::recv(void *buffer, int bufferSizeBytes, bool peek) {
	setNonblockingMode(false);

	int numBytesRead = ::recv(rawSocket, (char *)buffer, bufferSizeBytes, peek ? MSG_PEEK : 0);
	assert(numBytesRead == bufferSizeBytes);
}

void Socket::setNonblockingMode(bool enabled) {
	if (nonblockingModeEnabled == enabled)
		return;

	unsigned long enableNonblocking = enabled ? 1 : 0;
	char outputBuffer[256];
	DWORD bytesPutInOutputBuffer = 0;
	if (WSAIoctl(rawSocket, FIONBIO, &enableNonblocking, sizeof(unsigned long), outputBuffer, sizeof(outputBuffer), &bytesPutInOutputBuffer, NULL, NULL) != 0) {
		std::cout << GetLastErrorMessage(GetLastError()) << std::endl;
		assert(false);
	}
	nonblockingModeEnabled = enabled;
}

inline bool Socket::isNonblockingModeEnabled() const { return nonblockingModeEnabled; }

Socket *Socket::acceptFromMe() {
	setNonblockingMode(false);

	struct sockaddr fromAddress;
	int fromAddressLength = sizeof(sockaddr);
	SOCKET newRawSocketImpl = accept(rawSocket, &fromAddress, &fromAddressLength);
	if (newRawSocketImpl == INVALID_SOCKET) {
		std::cout << GetLastErrorMessage(WSAGetLastError(), true) << std::endl;
		assert(false);
	}
	assert(newRawSocketImpl != INVALID_SOCKET);

	return trackedNew(Socket(newRawSocketImpl));
}

Socket::Socket(SOCKET socket_) :
		rawSocket(socket_),
	nonblockingModeEnabled(false) {
}

Socket::~Socket() {
	closesocket(rawSocket);
}

void Socket::enterListenMode() {
	setNonblockingMode(true);
	int result = ::listen(rawSocket, 3);
	assert(result == 0);
}

void Socket::read(void *buffer, int bufferSizeBytes) {
	recv(buffer, bufferSizeBytes, false);
}

void Socket::peek(void *buffer, int bufferSizeBytes) {
	recv(buffer, bufferSizeBytes, true);
}

void Socket::send(void *buffer, int bufferSizeBytes) {
	::send(rawSocket, (const char *)buffer, bufferSizeBytes, 0);
}

Socket *Socket::openAndBind(const std::list<int> &listenPortOptions) {
	SOCKET socket = 0;

	if ((socket = ::socket(AF_INET, SOCK_STREAM, 0)) < 0)
		assert(false);

	bool foundPort = false;
	for (std::list<int>::const_iterator i = listenPortOptions.begin(),
			iEnd = listenPortOptions.end(); !foundPort && i != iEnd; i++) {
		int listenPort = *i;

		struct sockaddr_in name;
		name.sin_family = AF_INET;
		name.sin_addr.s_addr = htonl(INADDR_ANY);
		name.sin_port = htons(listenPort);
		int nameStructLength = sizeof(name);

		if (bind(socket, (struct sockaddr *)&name, nameStructLength)) {
			int error = GetLastError();
			if (error != WSAEADDRINUSE) {
				std::cout << "Error in connection listener: " << GetLastErrorMessage(error, true) << std::endl;
				assert(false);
			}
		}
		else
			foundPort = true;
	}


	Socket *result = trackedNew(Socket(socket));


	return result;
}

Socket *Socket::createConnectionTo(const char *hostName, const std::list<int> &portOptions) {
	SOCKET rawSocket = ::socket(AF_INET, SOCK_STREAM, 0);

	struct hostent *hp = gethostbyname(hostName);
	assert(hp);

	for (std::list<int>::const_iterator i = portOptions.begin(),
			iEnd = portOptions.end(); i != iEnd; i++) {
		int port = *i;

		struct sockaddr_in server;
		server.sin_family = AF_INET;
		memcpy(&server.sin_addr.s_addr, hp->h_addr, hp->h_length);
		server.sin_port = htons(port);

		if (::connect(rawSocket, (const sockaddr *)&server, sizeof(server))) {
			std::cout << GetLastErrorMessage(GetLastError(), true) << std::endl;
			assert(false);
		}
		else {
			return trackedNew(Socket(rawSocket));
		}
	}

	assert(false);
}

int Socket::getListeningPort() {
	struct sockaddr_in sin;
	int len = sizeof(sin);
	int success = getsockname(rawSocket, (struct sockaddr *)&sin, &len);
	assert(success != -1);

	return ntohs(sin.sin_port);
}
