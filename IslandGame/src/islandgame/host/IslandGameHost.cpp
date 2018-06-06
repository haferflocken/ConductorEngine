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
	const NavMeshTriangle triangle;

	Collection::Vector<NavMeshTriangleID> ids;
	for (size_t i = 0; i < 1000; ++i)
	{
		const Collection::Pair<uint32_t, NavMeshTriangleID> indexAndID = navMesh.AddTriangle(triangle);
		ids.Add(indexAndID.second);
	}
	for (size_t i = 0; i < 1000; ++i)
	{
		const uint32_t indexOfID = navMesh.FindIndexOfID(ids[i]);
		NavMeshConnections& connections = navMesh.GetConnectionsByIndex(indexOfID);

		NavMeshConnection& connectionA = connections.m_connections[connections.m_numConnections++];
		NavMeshConnection& connectionB = connections.m_connections[connections.m_numConnections++];
		NavMeshConnection& connectionC = connections.m_connections[connections.m_numConnections++];

		connectionA.m_connectedID = ids[rand() % ids.Size()];
		connectionB.m_connectedID = ids[rand() % ids.Size()];
		connectionC.m_connectedID = ids[rand() % ids.Size()];
	}

	const NavMeshTriangleID& startID = ids[rand() % ids.Size()];
	const NavMeshTriangleID& goalID = ids[rand() % ids.Size()];

	Collection::Vector<NavMeshTriangleID> path;
	const bool pathFound = AStarSearch(NavMeshGraphInterface(navMesh), startID, goalID, path);

	int x = 0;
	++x;
}
