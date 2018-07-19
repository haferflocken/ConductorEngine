#pragma once

#include <client/IClient.h>
#include <ecs/EntityManager.h>

namespace IslandGame { class IslandGameData; }

namespace IslandGame::Client
{
class IslandGameClient : public ::Client::IClient
{
	const IslandGameData& m_gameData;
	ECS::EntityManager m_entityManager;

public:
	IslandGameClient(const IslandGameData& gameData, ::Client::ConnectedHost& connectedHost);

	void Update() override;
};
}
