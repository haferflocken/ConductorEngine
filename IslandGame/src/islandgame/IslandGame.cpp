#include <islandgame/IslandGameData.h>

#include <behave/ActorInfoManager.h>
#include <behave/ActorManager.h>
#include <behave/BehaveContext.h>
#include <behave/BehaviourSystem.h>
#include <behave/components/SceneTransformComponent.h>
#include <behave/components/SceneTransformComponentInfo.h>

#include <collection/ProgramParameters.h>

#include <file/Path.h>

#include <client/ClientWorld.h>
#include <client/IClient.h>
#include <host/IHost.h>
#include <host/HostWorld.h>

#include <vulkanrenderer/VulkanInstance.h>

#include <iostream>

namespace Internal_IslandGame
{
constexpr char* k_dataDirectoryParameter = "-datapath";

constexpr char* k_vertexShaderPath = "shaders/vertex_shader.glsl";
constexpr char* k_fragmentShaderPath = "shaders/fragment_shader.glsl";

constexpr char* k_behaviourTreesPath = "behaviour_trees";
constexpr char* k_actorInfosPath = "actor_infos";
}

int main(const int argc, const char* argv[])
{
	using namespace Internal_IslandGame;
	using namespace IslandGame;

	// Extract the data directory from the command line parameters.
	const auto params = Collection::ProgramParameters(argc, argv);
	std::string dataDirectoryStr;
	if (!params.TryGet(k_dataDirectoryParameter, dataDirectoryStr))
	{
		std::cerr << "Missing required parameter: -datapath <dir>" << std::endl;
		return -1;
	}

	const File::Path dataDirectory = File::MakePath(dataDirectoryStr.c_str());

	// Find the vertex and fragment shaders in the data directory.
	const File::Path vertexShaderFile = dataDirectory / k_vertexShaderPath;
	const File::Path fragmentShaderFile = dataDirectory / k_fragmentShaderPath;

	// Load data files.
	IslandGameData gameData;
	gameData.LoadBehaviourTreesInDirectory(dataDirectory / k_behaviourTreesPath);
	gameData.LoadActorInfosInDirectory(dataDirectory / k_actorInfosPath);

	// Create and run a host and client.
	Host::HostWorld::HostFactory hostFactory = []() { return Mem::UniquePtr<Host::IHost>(); };
	Client::ClientWorld::ClientFactory clientFactory =
		[](Client::ConnectedHost&) { return Mem::UniquePtr<Client::IClient>(); };

	Host::HostWorld hostWorld{ std::move(hostFactory) };
	Client::ClientWorld clientWorld{ std::move(clientFactory) };

	// Connect the client to the host.



	/*{
		using namespace Behave;

		class TestSystem : public BehaviourSystemTempl<Util::TypeList<Behave::Components::SceneTransformComponent>, Util::TypeList<>>
		{
		public:
			void Update(ActorManager& actorManager, const BehaveContext& context,
				const Collection::ArrayView<ActorComponentGroupType>& components,
				Collection::Vector<std::function<void()>>& deferredFunctions) const {}
		};
		
		ActorManager actorManager{ gameData.GetActorComponentFactory() };
		actorManager.RegisterBehaviourSystem(Mem::MakeUnique<TestSystem>());

		actorManager.CreateActor(*gameData.GetActorInfoManager().FindActorInfo(Util::CalcHash("islander.json")));
		
		const BehaveContext context{ gameData.GetBehaviourTreeManager() };
		for (size_t i = 0; i < 10; ++i)
		{
			actorManager.Update(context);
		}
	}*/

	return 0;
}
