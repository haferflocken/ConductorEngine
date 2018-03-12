#include <behave/BehaviourTree.h>
#include <behave/BehaviourNode.h>
#include <behave/BehaviourNodeFactory.h>

#include <json/JSONTypes.h>

using namespace Behave;

BehaviourTree::BehaviourTree()
	: m_root()
{}

BehaviourTree::BehaviourTree(BehaviourTree&& o)
	: m_root(std::move(o.m_root))
{}

void BehaviourTree::operator=(BehaviourTree&& rhs)
{
	m_root = std::move(rhs.m_root);
}

BehaviourTree::~BehaviourTree()
{}

bool BehaviourTree::LoadFromJSON(const BehaviourNodeFactory& nodeFactory, const JSON::JSONObject& jsonObject)
{
	m_root = nodeFactory.MakeNode(jsonObject, *this);
	return (m_root != nullptr);
}
