#include <gamebase/ClientWorld.h>

#include <gamebase/ConnectedHost.h>
#include <gamebase/IClient.h>

using namespace GameBase;

ClientWorld::ClientWorld(ClientFactory&& clientFactory)
	: m_clientFactory(std::move(clientFactory))
	, m_connectedHost()
	, m_client()
{}

ClientWorld::~ClientWorld()
{
}
