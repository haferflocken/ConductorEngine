#include <behave/BehaviourTree.h>
#include <behave/BehaviourNode.h>
#include <behave/BehaviourNodeFactory.h>
#include <behave/parse/BehaveParsedTree.h>

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

bool Behave::BehaviourTree::LoadFromParsedTree(
	const BehaviourNodeFactory& nodeFactory,
	const Parse::ParsedTree& parsedTree)
{
	m_root = nodeFactory.MakeNode(parsedTree.m_rootNode, *this);
	return (m_root != nullptr);
}
