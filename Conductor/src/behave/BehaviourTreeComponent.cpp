#include <behave/BehaviourTreeComponent.h>

#include <behave/BehaviourNode.h>
#include <behave/BehaviourTree.h>
#include <ecs/ComponentVector.h>

namespace Behave
{
const Util::StringHash BehaviourTreeComponent::k_typeHash = Util::CalcHash(k_typeName);

void BehaviourTreeComponent::FullySerialize(
	const BehaviourTreeComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	// TODO(info) serialize
}

void BehaviourTreeComponent::ApplyFullSerialization(Asset::AssetManager& assetManager,
	BehaviourTreeComponent& component,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	// TODO(info) behaviour trees
	/*component.m_referencedForests.AddAll(componentInfo.m_behaviourForests.GetConstView());
	component.m_referencedForests.AddAll(componentInfo.m_importedForests.GetConstView());

	for (const Behave::BehaviourTree* const behaviourTree : componentInfo.m_behaviourTrees)
	{
		Behave::BehaviourTreeEvaluator& treeEvaluator = component.m_treeEvaluators.Emplace();
		behaviourTree->GetRoot()->PushState(treeEvaluator);
	}*/
}
}
