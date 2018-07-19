#pragma once

#include <client/IClient.h>
#include <ecs/ActorManager.h>

namespace IslandGame { class IslandGameData; }

namespace IslandGame::Client
{
class IslandGameClient : public ::Client::IClient
{
	const IslandGameData& m_gameData;
	ECS::ActorManager m_actorManager;

public:
	IslandGameClient(const IslandGameData& gameData, ::Client::ConnectedHost& connectedHost);

	void Update() override;
};
}
