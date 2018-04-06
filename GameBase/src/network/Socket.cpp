#include <network/Socket.h>

#include <dev/Dev.h>

// Winsock includes and library.
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")

bool Network::TryInitializeSocketAPI()
{
	WSAData wsaData;
	int errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (errorCode != 0)
	{
		Dev::LogError("WSAStartup() failed with error code [%d].", errorCode);
		return false;
	}
	Dev::Log("%s", wsaData.szDescription);
	return true;
}

void Network::ShutdownSocketAPI()
{
	WSACleanup();
}

struct Network::Socket::SocketImpl
{
	// Winsock socket.
	SOCKET m_platformSocket{ INVALID_SOCKET };
};

Network::Socket::Socket()
	: m_impl(Mem::MakeUnique<SocketImpl>())
{
}

Network::Socket::~Socket()
{
	if (m_impl->m_platformSocket != INVALID_SOCKET)
	{
		Close();
	}
}

Network::Socket::Socket(Socket&& other)
	: m_impl(std::move(other.m_impl))
{
	other.m_impl = Mem::MakeUnique<SocketImpl>();
}

Network::Socket& Network::Socket::operator=(Socket&& rhs)
{
	m_impl = std::move(rhs.m_impl);
	rhs.m_impl = Mem::MakeUnique<SocketImpl>();
	return *this;
}

bool Network::Socket::TryListen()
{
	if (listen(m_impl->m_platformSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		Dev::LogError("listen() failed with error code [%d].", WSAGetLastError());
		closesocket(m_impl->m_platformSocket);
		m_impl->m_platformSocket = INVALID_SOCKET; 
		return false;
	}
	return true;
}

Network::Socket Network::Socket::Accept()
{
	SOCKET clientSocket = accept(m_impl->m_platformSocket, nullptr, nullptr);
	if (clientSocket == INVALID_SOCKET)
	{
		Dev::LogError("accept() failed with error code [%d].", WSAGetLastError());
		closesocket(clientSocket);
		return Socket();
	}
	
	// Return the platform specific socket wrapped in a platform agnostic way.
	Socket outSocket;
	outSocket.GetImpl().m_platformSocket = clientSocket;
	return outSocket;
}

void Network::Socket::Close()
{
	closesocket(m_impl->m_platformSocket);
	m_impl->m_platformSocket = INVALID_SOCKET;
}

Network::Socket Network::CreateAndBindListenerSocket(const char* port)
{
	addrinfo* result = nullptr;
	addrinfo* ptr = nullptr;
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	const int errorCode = getaddrinfo(NULL, port, &hints, &result);
	if (errorCode != 0)
	{
		Dev::LogError("getaddrinfo() failed with error code [%d].", errorCode);
		return Socket();
	}

	// Create a listener socket.
	SOCKET listenerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenerSocket == INVALID_SOCKET)
	{
		Dev::LogError("socket() failed with error code [%d].", WSAGetLastError());
		freeaddrinfo(result);
		return Socket();
	}

	// Bind the socket.
	if (bind(listenerSocket, result->ai_addr, static_cast<int>(result->ai_addrlen)) == SOCKET_ERROR)
	{
		Dev::LogError("bind() failed with error code [%d].", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenerSocket);
		return Socket();
	}

	// Free the addrinfo after it is no longer needed.
	freeaddrinfo(result);

	// Return the platform specific socket wrapped in a platform agnostic way.
	Socket outSocket;
	outSocket.GetImpl().m_platformSocket = listenerSocket;
	return outSocket;
}
