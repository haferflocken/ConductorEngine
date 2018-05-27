#pragma once

#include <behave/ActorManager.h>
#include <client/IClient.h>

namespace IslandGame { class IslandGameData; }

namespace IslandGame::Client
{
class IslandGameClient : public ::Client::IClient
{
	const IslandGameData& m_gameData;
	Behave::ActorManager m_actorManager;

public:
	IslandGameClient(const IslandGameData& gameData, ::Client::ConnectedHost& connectedHost);

	void Update() override;
};
}
