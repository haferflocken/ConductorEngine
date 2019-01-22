#include <input/InputComponent.h>

#include <ecs/ComponentVector.h>
#include <json/JSONTypes.h>

namespace Input
{
namespace Internal_InputComponent
{
const Util::StringHash k_inputNamesHash = Util::CalcHash("input_names");
}

const Util::StringHash InputComponent::k_typeHash = Util::CalcHash(k_typeName);

bool InputComponent::TryCreateFromInfo(Asset::AssetManager& assetManager,
	const ECS::ComponentID reservedID, ECS::ComponentVector& destination)
{
	InputComponent& inputComponent = destination.Emplace<InputComponent>(reservedID);
	/*for (const auto& nameHash : componentInfo.m_inputNameHashes)
	{
		inputComponent.m_inputMap[nameHash];
	}*/
	return true;
}
}
