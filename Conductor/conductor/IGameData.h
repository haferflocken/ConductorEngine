#pragma once

#include <file/Path.h>
#include <mem/UniquePtr.h>

#include <functional>

namespace Asset { class AssetManager; }

namespace Behave
{
class BehaviourNodeFactory;
}

namespace Behave::AST
{
class Interpreter;
}

namespace ECS
{
class ComponentReflector;
}

namespace Conductor
{
/**
 * IGameData is the base class that any game data storage should extend.
 * Game-specific subclasses should register their ECS types in their constructor.
 */
class IGameData
{
protected:
	// Protected constructor so that games must extend this with their own type.
	explicit IGameData(const File::Path& dataDirectory, const File::Path& userDirectory,
		Asset::AssetManager& assetManager);

public:
	virtual ~IGameData();

	const File::Path& GetDataDirectory() const { return m_dataDirectory; }
	const File::Path& GetUserDirectory() const { return m_userDirectory; }

	// Requesting assets modifies the AssetManager, so it cannot be const.
	Asset::AssetManager& GetAssetManager() const { return m_assetManager; }

	ECS::ComponentReflector& GetComponentReflector() { return *m_componentReflector; }
	const ECS::ComponentReflector& GetComponentReflector() const { return *m_componentReflector; }

	const Behave::AST::Interpreter& GetBehaveASTInterpreter() const { return *m_behaveASTInterpreter; }
	const Behave::BehaviourNodeFactory& GetBehaviourNodeFactory() const { return *m_behaviourNodeFactory; }

protected:
	File::Path m_dataDirectory;
	File::Path m_userDirectory;

	Asset::AssetManager& m_assetManager;

	Mem::UniquePtr<ECS::ComponentReflector> m_componentReflector;

	Mem::UniquePtr<Behave::AST::Interpreter> m_behaveASTInterpreter;
	Mem::UniquePtr<Behave::BehaviourNodeFactory> m_behaviourNodeFactory;
};

using GameDataFactory = std::function<Mem::UniquePtr<IGameData>(Asset::AssetManager&, const File::Path&, const File::Path&)>;
}
