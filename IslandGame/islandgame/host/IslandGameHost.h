#pragma once

#include <behave/ActorManager.h>
#include <host/IHost.h>

namespace IslandGame { class IslandGameData; }

namespace IslandGame::Host
{
class IslandGameHost : public ::Host::IHost
{
	const IslandGameData& m_gameData;
	Behave::ActorManager m_actorManager;

public:
	IslandGameHost(const IslandGameData& gameData);
	~IslandGameHost();

	void Update() override;
};
}
