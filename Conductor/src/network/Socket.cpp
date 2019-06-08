#include <network/Socket.h>

#include <dev/Dev.h>

// Winsock includes and library.
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")

namespace Internal_Socket
{
void LogWinsockError(const char* message, int errorCode)
{
#define LOGSOCKERR(ERRSTR) AMP_LOG("%s%s", message, ERRSTR); break
	switch (errorCode)
	{
	case WSA_INVALID_HANDLE:	LOGSOCKERR("WSA_INVALID_HANDLE");
	case WSA_NOT_ENOUGH_MEMORY:	LOGSOCKERR("WSA_NOT_ENOUGH_MEMORY");
	case WSA_INVALID_PARAMETER:	LOGSOCKERR("WSA_INVALID_PARAMETER");
	case WSA_OPERATION_ABORTED:	LOGSOCKERR("WSA_OPERATION_ABORTED");
	case WSA_IO_INCOMPLETE:		LOGSOCKERR("WSA_IO_INCOMPLETE");
	case WSA_IO_PENDING:		LOGSOCKERR("WSA_IO_PENDING");
	case WSAEINTR:				LOGSOCKERR("WSAEINTR");
	case WSAEBADF:				LOGSOCKERR("WSAEBADF");
	case WSAEACCES:				LOGSOCKERR("WSAEACCES");
	case WSAEFAULT:				LOGSOCKERR("WSAEFAULT");
	case WSAEINVAL:				LOGSOCKERR("WSAEINVAL");
	case WSAEMFILE:				LOGSOCKERR("WSAEMFILE");
	case WSAEWOULDBLOCK:		LOGSOCKERR("WSAEWOULDBLOCK");
	case WSAEINPROGRESS:		LOGSOCKERR("WSAEINPROGRESS");
	case WSAEALREADY:			LOGSOCKERR("WSAEALREADY");
	case WSAENOTSOCK:			LOGSOCKERR("WSAENOTSOCK");
	case WSAEDESTADDRREQ:		LOGSOCKERR("WSAEDESTADDRREQ");
	case WSAEMSGSIZE:			LOGSOCKERR("WSAEMSGSIZE");
	case WSAEPROTOTYPE:			LOGSOCKERR("WSAEPROTOTYPE");
	case WSAENOPROTOOPT:		LOGSOCKERR("WSAENOPROTOOPT");
	case WSAEPROTONOSUPPORT:	LOGSOCKERR("WSAEPROTONOSUPPORT");
	case WSAESOCKTNOSUPPORT:	LOGSOCKERR("WSAESOCKTNOSUPPORT");
	case WSAEOPNOTSUPP:			LOGSOCKERR("WSAEOPNOTSUPP");
	case WSAEPFNOSUPPORT:		LOGSOCKERR("WSAEPFNOSUPPORT");
	case WSAEAFNOSUPPORT:		LOGSOCKERR("WSAEAFNOSUPPORT");
	case WSAEADDRINUSE:			LOGSOCKERR("WSAEADDRINUSE");
	case WSAEADDRNOTAVAIL:		LOGSOCKERR("WSAEADDRNOTAVAIL");
	case WSAENETDOWN:			LOGSOCKERR("WSAENETDOWN");
	case WSAENETUNREACH:		LOGSOCKERR("WSAENETUNREACH");
	case WSAENETRESET:			LOGSOCKERR("WSAENETRESET");
	case WSAECONNABORTED:		LOGSOCKERR("WSAECONNABORTED");
	case WSAECONNRESET:			LOGSOCKERR("WSAECONNRESET");
	default:					AMP_LOG("%sEC[%d]", message, errorCode); break;
	}
#undef LOGSOCKERR
}
}

bool Network::TryInitializeSocketAPI()
{
	WSAData wsaData;
	int errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (errorCode != 0)
	{
		AMP_LOG_ERROR("WSAStartup() failed with error code [%d].", errorCode);
		return false;
	}
	AMP_LOG("%s", wsaData.szDescription);
	return true;
}

void Network::ShutdownSocketAPI()
{
	WSACleanup();
}

struct Network::Socket::SocketImpl
{
	SocketImpl() = default;

	explicit SocketImpl(SOCKET platformSocket)
		: m_platformSocket(platformSocket)
		, m_maxMessageBytes(0)
	{
		DWORD maxMessageSize;
		int sizeOfMaxMessageSize = sizeof(maxMessageSize);
		const int errorCode = getsockopt(
			m_platformSocket,
			SOL_SOCKET,
			SO_MAX_MSG_SIZE,
			reinterpret_cast<char*>(&maxMessageSize),
			&sizeOfMaxMessageSize);
		if (errorCode != SOCKET_ERROR)
		{
			m_maxMessageBytes = static_cast<size_t>(maxMessageSize);
		}
	}

	// Winsock socket.
	SOCKET m_platformSocket{ INVALID_SOCKET };
	size_t m_maxMessageBytes{ 0 };
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

Network::Socket::Socket(Socket&& other) noexcept
	: m_impl(std::move(other.m_impl))
{
	other.m_impl = Mem::MakeUnique<SocketImpl>();
}

Network::Socket& Network::Socket::operator=(Socket&& rhs) noexcept
{
	m_impl = std::move(rhs.m_impl);
	rhs.m_impl = Mem::MakeUnique<SocketImpl>();
	return *this;
}

bool Network::Socket::IsValid() const
{
	return m_impl->m_platformSocket != INVALID_SOCKET;
}

bool Network::Socket::TryListen()
{
	if (listen(m_impl->m_platformSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		Internal_Socket::LogWinsockError("listen() failed: ", WSAGetLastError());
		closesocket(m_impl->m_platformSocket);
		*m_impl = SocketImpl(); 
		return false;
	}
	return true;
}

Network::Socket Network::Socket::Accept()
{
	SOCKET clientSocket = accept(m_impl->m_platformSocket, nullptr, nullptr);
	if (clientSocket == INVALID_SOCKET)
	{
		Internal_Socket::LogWinsockError("accept() failed: ", WSAGetLastError());
		closesocket(clientSocket);
		return Socket();
	}
	
	// Return the platform specific socket wrapped in a platform agnostic way.
	Socket outSocket;
	(*outSocket.m_impl) = SocketImpl(clientSocket);
	return outSocket;
}

size_t Network::Socket::AcceptPendingConnections(Socket* outSockets, const size_t maxAcceptCount)
{
	fd_set readSockets;
	readSockets.fd_count = 1;
	readSockets.fd_array[0] = m_impl->m_platformSocket;

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	size_t i = 0;
	while (i < maxAcceptCount)
	{
		const int result = select(0, &readSockets, nullptr, nullptr, &timeout);
		if (result == SOCKET_ERROR)
		{
			Internal_Socket::LogWinsockError("select() failed: ", WSAGetLastError());
			return i;
		}
		if (result == 0)
		{
			return i;
		}

		outSockets[i++] = Accept();
	}
	return i;
}

void Network::Socket::Close()
{
	closesocket(m_impl->m_platformSocket);
	*m_impl = SocketImpl();
}

void Network::Socket::Send(const Collection::ArrayView<const uint8_t>& bytes)
{
	AMP_FATAL_ASSERT(IsValid(), "Can't send from an invalid socket!");

	if (bytes.Size() > m_impl->m_maxMessageBytes)
	{
		AMP_LOG_ERROR("Can't send message of size [%zu] because it exceeds maximum size [%zu].",
			bytes.Size(), m_impl->m_maxMessageBytes);
		return;
	}

	const int numBytesSent = send(
		m_impl->m_platformSocket,
		reinterpret_cast<const char*>(bytes.begin()),
		static_cast<int>(bytes.Size()),
		0);
	if (numBytesSent == SOCKET_ERROR)
	{
		Internal_Socket::LogWinsockError("send() failed: ", WSAGetLastError());
		Close();
		return;
	}
	if (numBytesSent != bytes.Size())
	{
		AMP_LOG_ERROR("Socket::Send() didn't send all the bytes it was given!");
		return;
	}
}

size_t Network::Socket::Receive(Collection::ArrayView<uint8_t>& outBytes)
{
	AMP_FATAL_ASSERT(IsValid(), "Can't receive on an invalid socket!");

	fd_set toRead;
	toRead.fd_count = 1;
	toRead.fd_array[0] = m_impl->m_platformSocket;

	TIMEVAL timeOut;
	timeOut.tv_sec = 0;
	timeOut.tv_usec = 100;

	const int selectResult = select(0, &toRead, nullptr, nullptr, &timeOut);
	if (selectResult == 0)
	{
		return 0;
	}
	if (selectResult == SOCKET_ERROR)
	{
		Internal_Socket::LogWinsockError("select() failed: ", WSAGetLastError());
		Close();
		return 0;
	}

	const int numBytesReceived = recv(
		m_impl->m_platformSocket,
		reinterpret_cast<char*>(outBytes.begin()),
		static_cast<int>(outBytes.Size()),
		0);
	if (numBytesReceived == SOCKET_ERROR)
	{
		Internal_Socket::LogWinsockError("recv() failed: ", WSAGetLastError());
		Close();
		return 0;
	}
	return static_cast<size_t>(numBytesReceived);
}

Network::Socket Network::CreateAndBindListenerSocket(const char* port)
{
	addrinfo* result = nullptr;
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	const int errorCode = getaddrinfo(NULL, port, &hints, &result);
	if (errorCode != 0)
	{
		Internal_Socket::LogWinsockError("getaddrinfo() failed: ", errorCode);
		return Socket();
	}

	// Create a listener socket.
	SOCKET listenerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenerSocket == INVALID_SOCKET)
	{
		Internal_Socket::LogWinsockError("socket() failed: ", WSAGetLastError());
		freeaddrinfo(result);
		return Socket();
	}

	// Bind the socket.
	if (bind(listenerSocket, result->ai_addr, static_cast<int>(result->ai_addrlen)) == SOCKET_ERROR)
	{
		Internal_Socket::LogWinsockError("bind() failed: ", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenerSocket);
		return Socket();
	}

	// Free the addrinfo after it is no longer needed.
	freeaddrinfo(result);

	// Return the platform specific socket wrapped in a platform agnostic way.
	Socket outSocket;
	outSocket.GetImpl() = Socket::SocketImpl(listenerSocket);
	return outSocket;
}

Network::Socket Network::CreateConnectedSocket(const char* hostName, const char* port)
{
	addrinfo* result = nullptr;
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	const int errorCode = getaddrinfo(hostName, port, &hints, &result);
	if (errorCode != 0)
	{
		Internal_Socket::LogWinsockError("getaddrinfo() failed: ", errorCode);
		return Socket();
	}

	// Create a socket to connect to the host.
	addrinfo* ptr = result;
	SOCKET connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (connectSocket == INVALID_SOCKET)
	{
		Internal_Socket::LogWinsockError("socket() failed: ", WSAGetLastError());
		freeaddrinfo(result);
		return Socket();
	}

	// Connect the socket to the host.
	if (connect(connectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen)) == SOCKET_ERROR)
	{
		Internal_Socket::LogWinsockError("connect() failed: ", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(connectSocket);
		return Socket();
	}

	// TODO(network) try to connect to all the results rather than just the first

	// Free the addrinfo after it is no longer needed.
	freeaddrinfo(result);
	
	// Return the platform specific socket wrapped in a platform agnostic way.
	Socket outSocket;
	outSocket.GetImpl() = Socket::SocketImpl(connectSocket);
	return outSocket;
}
