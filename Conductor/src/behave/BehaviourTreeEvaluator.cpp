#include <behave/BehaviourTreeEvaluator.h>

#include <behave/BehaveContext.h>
#include <behave/BehaviourCondition.h>
#include <behave/BehaviourNode.h>
#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTree.h>
#include <behave/nodes/ReturnNode.h>

const Behave::BehaviourTree* Behave::BehaviourTreeEvaluator::GetCurrentTree() const
{
	return (!m_callStack.IsEmpty()) ? &m_callStack.Peek()->GetNode()->GetTree() : nullptr;
}

void Behave::BehaviourTreeEvaluator::Update(
	ECS::Entity& entity,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions,
	const BehaveContext& context)
{
	Dev::FatalAssert(!m_callStack.IsEmpty(), "Cannot update without a call stack.");

	// Check the active domains and make sure they are still valid.
	// If a domain fails, unwind the call stack to the domain node and notify it of its failure.
	for (const auto& domainEntry : m_domainStack)
	{
		// Make copies of the entry values as they may become invalidated when popping nodes off the call stack,
		// as early-terminating a domain node pops its domain from the domain stack, causing iterator invalidation.
		const BehaviourNodeState* const domainNodeState = domainEntry.m_state;
		const BehaviourCondition* const domainCondition = domainEntry.m_condition;

		if (!domainCondition->Check(context.m_interpreter, context.m_entityManager, entity))
		{
			while (domainNodeState != m_callStack.Peek())
			{
				m_callStack.Peek()->NotifyEarlyTermination(*this);
				m_callStack.Pop();
			}
			m_callStack.Peek()->NotifyChildFinished(nullptr, EvaluateResult::Running);
			break;
		}
	}

	// Update the node states.
	BehaviourNodeState* nodeState = m_callStack.Peek();
	EvaluateResult result = nodeState->Evaluate(entity, *this, deferredFunctions, context);
	while (result != EvaluateResult::Running)
	{
		switch (result)
		{
		case EvaluateResult::PushedNode:
		{
			// Immediately evaluate the pushed node.
			nodeState = m_callStack.Peek();
			result = nodeState->Evaluate(entity, *this, deferredFunctions, context);
			break;
		}
		case EvaluateResult::Success:
		case EvaluateResult::Failure:
		{
			// Pop this node.
			const BehaviourNode* const finishedNode = nodeState->GetNode();
			m_callStack.Pop();

			// If there are no nodes left on the stack, return.
			if (m_callStack.IsEmpty())
			{
				return;
			}

			// Otherwise, immediately evaluate its parent.
			nodeState = m_callStack.Peek();
			nodeState->NotifyChildFinished(finishedNode, result);
			result = nodeState->Evaluate(entity, *this, deferredFunctions, context);
			break;
		}
		case EvaluateResult::Return:
		{
			// Pop node states until at the previous tree.
			Dev::FatalAssert(dynamic_cast<const Nodes::ReturnNode*>(nodeState->GetNode()) != nullptr,
				"Only ReturnBehaviourState should ever result in EvaluateResult::Return.");

			const Nodes::ReturnNode* const finishedNode = static_cast<const Nodes::ReturnNode*>(nodeState->GetNode());
			const BehaviourTree& finishedTree = finishedNode->GetTree();
			m_callStack.Pop();
			while (!m_callStack.IsEmpty() && (&m_callStack.Peek()->GetNode()->GetTree() == &finishedTree))
			{
				m_callStack.Pop();
			}
			nodeState = m_callStack.Peek();

			// Notify the caller that the tree finished, but do not reevaluate the caller.
			nodeState->NotifyChildFinished(finishedNode,
				finishedNode->ReturnsSuccess() ? EvaluateResult::Success : EvaluateResult::Failure);
			return;
		}
		default:
		{
			Dev::FatalError("Unknown evaluate result [%d].", static_cast<int32_t>(result));
			return;
		}
		}
	}
}
