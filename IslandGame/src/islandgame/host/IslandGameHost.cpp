#include <islandgame/host/IslandGameHost.h>

#include <islandgame/IslandGameData.h>

#include <behave/BehaveContext.h>
#include <ecs/EntityInfoManager.h>

#include <navigation/AStar.h>
#include <navigation/NavMesh.h>
#include <navigation/NavMeshGraphInterface.h>

namespace Internal_IslandGameHost
{
Navigation::NavMesh MakeTestNavMesh()
{
	using namespace Navigation;

	// TODO not a random navmesh 

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

	return navMesh;
}
}

IslandGame::Host::IslandGameHost::IslandGameHost(const IslandGameData& gameData)
	: m_gameData(gameData)
	, m_navigationManager(Internal_IslandGameHost::MakeTestNavMesh())
	, m_entityManager(gameData.GetComponentFactory())
{
	// TODO create this at the right place
	const Navigation::NavigatorID navigatorID = m_navigationManager.CreateNavigator(
		Math::Vector3(), Math::Vector3(1.0f, 0.0f, 0.0f));

	m_entityManager.CreateEntity(*m_gameData.GetEntityInfoManager().FindEntityInfo(Util::CalcHash("islander.json")));
}

IslandGame::Host::IslandGameHost::~IslandGameHost()
{
}

void IslandGame::Host::IslandGameHost::Update()
{
	m_navigationManager.Update();

	const Behave::BehaveContext context{ m_gameData.GetBehaviourTreeManager() };
	m_entityManager.Update(context);
}
