#pragma once

#include <client/IClient.h>

namespace IslandGame::Client
{
class IslandGameClient : public ::Client::IClient
{
public:
	IslandGameClient(::Client::ConnectedHost& connectedHost)
		: IClient(connectedHost)
	{}

	void Update() override;
};
}
