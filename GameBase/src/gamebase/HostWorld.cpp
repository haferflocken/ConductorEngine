#include <gamebase/HostWorld.h>

#include <gamebase/ConnectedClient.h>
#include <gamebase/IHost.h>

GameBase::HostWorld::HostWorld(HostFactory&& hostFactory)
	: m_hostFactory(std::move(hostFactory))
	, m_host()
	, m_connectedClients()
{
}

GameBase::HostWorld::~HostWorld()
{
}
