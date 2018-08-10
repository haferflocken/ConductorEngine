#pragma once

#include <ecs/EntityManager.h>
#include <host/IHost.h>
#include <navigation/NavigationManager.h>

namespace IslandGame { class IslandGameData; }

namespace IslandGame::Host
{
class IslandGameHost : public ::Host::IHost
{
	const IslandGameData& m_gameData;
	ECS::EntityManager m_entityManager;

public:
	IslandGameHost(const IslandGameData& gameData);
	~IslandGameHost();

	void Update() override;
};
}
