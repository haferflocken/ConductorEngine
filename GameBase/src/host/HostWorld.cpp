#include <host/HostWorld.h>

#include <host/ConnectedClient.h>
#include <host/IHost.h>

Host::HostWorld::HostWorld(HostFactory&& hostFactory)
	: m_hostFactory(std::move(hostFactory))
{
	m_hostThread = std::thread(&HostWorld::HostThreadFunction, this);
}

Host::HostWorld::~HostWorld()
{
	RequestShutdown();
	m_hostThread.join();
}

void Host::HostWorld::RequestShutdown()
{
	m_hostThreadStatus = HostThreadStatus::ShutdownRequested;
}

void Host::HostWorld::HostThreadFunction()
{
	m_hostThreadStatus = HostThreadStatus::Running;
	m_host = m_hostFactory();

	while (m_hostThreadStatus == HostThreadStatus::Running)
	{
		// TODO get pending client input
		// TODO tick game simulation
		// TODO transmit game state to clients
	}

	m_host.Reset();
	m_hostThreadStatus = HostThreadStatus::Stopped;
}
