#pragma once

#include <mem/UniquePtr.h>

namespace Network
{
bool TryInitializeSocketAPI();
void ShutdownSocketAPI();

class Socket
{
public:
	struct SocketImpl;

	Socket();
	~Socket();

	Socket(Socket&& other);
	Socket& operator=(Socket&& rhs);

	SocketImpl& GetImpl() { return *m_impl; }
	
	// Test if this socket object actually represents a network socket.
	bool IsValid() const;

	// Attempt to listen on this socket and return true if succesful.
	bool TryListen();
	// Accept a pending connection and return it. Blocks is no connection is available.
	Socket Accept();
	// Accept up to maxAcceptCount pending connections and place them in outSockets. Non-blocking.
	size_t AcceptPendingConnections(Socket* outSockets, const size_t maxAcceptCount);
	// Close the socket.
	void Close();

private:
	Mem::UniquePtr<SocketImpl> m_impl;
};

Socket CreateAndBindListenerSocket(const char* port);
Socket CreateConnectedSocket(const char* hostName, const char* port);
}
