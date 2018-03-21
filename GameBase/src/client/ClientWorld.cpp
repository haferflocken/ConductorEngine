#include <client/ClientWorld.h>

#include <client/ConnectedHost.h>
#include <client/IClient.h>

Client::ClientWorld::ClientWorld(ClientFactory&& clientFactory)
	: m_clientFactory(std::move(clientFactory))
	, m_connectedHost()
	, m_client()
{}

Client::ClientWorld::~ClientWorld()
{
}
