#pragma once

#include <ecs/ActorManager.h>
#include <host/IHost.h>
#include <navigation/NavigationManager.h>

namespace IslandGame { class IslandGameData; }

namespace IslandGame::Host
{
class IslandGameHost : public ::Host::IHost
{
	const IslandGameData& m_gameData;
	Navigation::NavigationManager m_navigationManager;
	ECS::ActorManager m_actorManager;

public:
	IslandGameHost(const IslandGameData& gameData);
	~IslandGameHost();

	void Update() override;
};
}
