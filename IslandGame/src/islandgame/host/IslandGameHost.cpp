#include <islandgame/host/IslandGameHost.h>

#include <islandgame/IslandGameData.h>

#include <behave/ActorInfoManager.h>
#include <behave/BehaveContext.h>

#include <navigation/AStar.h>
#include <navigation/NavMesh.h>
#include <navigation/NavMeshGraphInterface.h>

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

	using namespace Navigation;

	NavMesh navMesh;
	const NavMeshTriangle startTriangle{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f / 3.0f, 1.0f / 3.0f };
	const NavMeshTriangle goalTriangle{ 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, -1.0f / 3.0f, -1.0f / 3.0f };

	const Collection::Pair<uint32_t, NavMeshTriangleID> startIndexAndID = navMesh.AddTriangle(startTriangle);
	const Collection::Pair<uint32_t, NavMeshTriangleID> goalIndexAndID = navMesh.AddTriangle(goalTriangle);

	NavMeshConnections& startConnections = navMesh.GetConnectionsByIndex(startIndexAndID.first);
	NavMeshConnection& connectionToGoal = startConnections.m_connections[startConnections.m_numConnections++];
	connectionToGoal.m_connectedID = goalIndexAndID.second;

	Collection::Vector<NavMeshTriangleID> path;
	const bool pathFound = AStarSearch(
		NavMeshGraphInterface(navMesh), startIndexAndID.second, goalIndexAndID.second, path);

	int x = 0;
	++x;
}
