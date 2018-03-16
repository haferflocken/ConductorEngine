#include <behave/BehaviourTree.h>
#include <behave/BehaviourNode.h>
#include <behave/BehaviourNodeFactory.h>

#include <json/JSONTypes.h>

Behave::BehaviourTree::BehaviourTree()
	: m_root()
{}

Behave::BehaviourTree::BehaviourTree(BehaviourTree&& o)
	: m_root(std::move(o.m_root))
{}

void Behave::BehaviourTree::operator=(BehaviourTree&& rhs)
{
	m_root = std::move(rhs.m_root);
}

Behave::BehaviourTree::~BehaviourTree()
{}

bool Behave::BehaviourTree::LoadFromJSON(const BehaviourNodeFactory& nodeFactory, const JSON::JSONObject& jsonObject)
{
	m_root = nodeFactory.MakeNode(jsonObject, *this);
	return (m_root != nullptr);
}
