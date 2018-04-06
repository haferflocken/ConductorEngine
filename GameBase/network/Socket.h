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

	bool TryListen();
	Socket Accept();
	void Close();

private:
	Mem::UniquePtr<SocketImpl> m_impl;
};

Socket CreateAndBindListenerSocket(const char* port);
}
