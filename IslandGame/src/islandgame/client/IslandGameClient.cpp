#include <islandgame/client/IslandGameClient.h>

#include <client/ConnectedHost.h>

void IslandGame::Client::IslandGameClient::Update()
{
	m_connectedHost.Disconnect();
}
