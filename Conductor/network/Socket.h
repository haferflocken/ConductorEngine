#pragma once

#include <collection/ArrayView.h>
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

	Socket(Socket&& other) noexcept;
	Socket& operator=(Socket&& rhs) noexcept;

	SocketImpl& GetImpl() { return *m_impl; }
	
	// Test if this socket object actually represents a network socket.
	bool IsValid() const;

	// Attempt to listen on this socket and return true if succesful.
	bool TryListen();
	// Accept a pending connection and return it. Blocks if no connection is available.
	Socket Accept();
	// Accept up to maxAcceptCount pending connections and place them in outSockets. Non-blocking.
	size_t AcceptPendingConnections(Socket* outSockets, const size_t maxAcceptCount);
	// Close the socket.
	void Close();

	// Send the given buffer to this socket's endpoint.
	void Send(const Collection::ArrayView<const uint8_t>& bytes);
	// Receive any data pending on this socket. Receives only up to outBytes.Size().
	// Returns the number of bytes read. Non-blocking.
	size_t Receive(Collection::ArrayView<uint8_t>& outBytes);

private:
	Mem::UniquePtr<SocketImpl> m_impl;
};

Socket CreateAndBindListenerSocket(const char* port);
Socket CreateConnectedSocket(const char* hostName, const char* port);
}
