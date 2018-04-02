#pragma once

namespace Client
{
class ConnectedHost;
struct InputMessage;

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

	virtual void NotifyOfWindowClosed() {};
	virtual void NotifyOfKeyUp(const char key) {};
	virtual void NotifyOfKeyDown(const char key) {};
	virtual void NotifyOfInputMessage(Client::InputMessage& message) {}
};
}
