#pragma once

#include <client/IClient.h>

namespace IslandGame { class IslandGameData; }

namespace IslandGame::Client
{
class IslandGameClient : public ::Client::IClient
{
	const IslandGameData& m_gameData;

public:
	IslandGameClient(const IslandGameData& gameData, ::Client::ConnectedHost& connectedHost);

	void Update(const Unit::Time::Millisecond delta) override;
};
}
