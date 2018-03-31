#include <islandgame/host/IslandGameHost.h>

#include <islandgame/IslandGameData.h>

#include <behave/ActorInfoManager.h>
#include <behave/BehaveContext.h>

IslandGame::Host::IslandGameHost::IslandGameHost(const IslandGameData& gameData)
	: m_gameData(gameData)
	, m_actorManager(gameData.GetActorComponentFactory())
{
	m_actorManager.CreateActor(*m_gameData.GetActorInfoManager().FindActorInfo(Util::CalcHash("islander.json")));
}

IslandGame::Host::IslandGameHost::~IslandGameHost()
{
}

void IslandGame::Host::IslandGameHost::Update()
{
	const Behave::BehaveContext context{ m_gameData.GetBehaviourTreeManager() };
	m_actorManager.Update(context);
}
