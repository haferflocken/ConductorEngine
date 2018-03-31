#pragma once

namespace Client
{
class ConnectedHost;

// IClient is the interface a game's client must implement.
class IClient
{
protected:
	ConnectedHost& m_connectedHost;

public:
	IClient(ConnectedHost& connectedHost)
		: m_connectedHost(connectedHost)
	{}

	virtual void Update() = 0;
};
}
