#pragma once

#include <host/IHost.h>

namespace IslandGame { class IslandGameData; }

namespace IslandGame::Host
{
class IslandGameHost : public ::Host::IHost
{
	const IslandGameData& m_gameData;

public:
	IslandGameHost(const IslandGameData& gameData);
	~IslandGameHost();

	void Update(const Unit::Time::Millisecond delta) override;
};
}
