#include <gamebase/ClientWorld.h>

#include <gamebase/ConnectedHost.h>
#include <gamebase/IClient.h>

GameBase::ClientWorld::ClientWorld(ClientFactory&& clientFactory)
	: m_clientFactory(std::move(clientFactory))
	, m_connectedHost()
	, m_client()
{}

GameBase::ClientWorld::~ClientWorld()
{
}
