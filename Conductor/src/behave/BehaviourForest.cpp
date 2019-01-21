#include <behave/BehaviourForest.h>

#include <behave/parse/BehaviourTreeParser.h>
#include <file/FullFileReader.h>

namespace Behave
{
bool BehaviourForest::TryLoad(const BehaviourNodeFactory& nodeFactory,
	const File::Path& filePath,
	BehaviourForest* destination)
{
	const std::string fileContents = File::ReadFullTextFile(filePath);
	Parse::ParseResult parseResult = Parse::Parser::ParseTrees(fileContents.c_str());

	Collection::Vector<std::string> imports;
	Collection::VectorMap<Util::StringHash, BehaviourTree> trees;
	parseResult.Match(
		[&](Parse::ParsedForest& parsedForest)
		{
			imports = std::move(parsedForest.m_imports);

			for (auto& parsedTree : parsedForest.m_parsedTrees)
			{
				BehaviourTree tree;
				if (tree.LoadFromParsedTree(nodeFactory, parsedTree))
				{
					trees[Util::CalcHash(parsedTree.m_treeName)] = std::move(tree);
				}
				else
				{
					AMP_LOG_WARNING("Failed to load behaviour tree \"%s\" from file [%S].",
						parsedTree.m_treeName.c_str(), filePath.c_str());
				}
			}
		},
		[&](const Parse::SyntaxError& syntaxError)
		{
			AMP_LOG_WARNING("%s Ln %d Ch %d: %s", filePath.string().c_str(), syntaxError.m_lineNumber,
				syntaxError.m_characterInLine, syntaxError.m_message.c_str());
		});

	if (!trees.IsEmpty())
	{
		destination = new(destination) BehaviourForest(std::move(imports), std::move(trees));
		return true;
	}
	return false;
}

BehaviourForest::BehaviourForest(Collection::Vector<std::string>&& imports,
	Collection::VectorMap<Util::StringHash, BehaviourTree>&& trees)
	: m_imports(std::move(imports))
	, m_trees(std::move(trees))
{}

BehaviourForest::~BehaviourForest() = default;

const BehaviourTree* BehaviourForest::FindTree(const Util::StringHash treeNameHash) const
{
	const auto itr = m_trees.Find(treeNameHash);
	return (itr != m_trees.end()) ? &itr->second : nullptr;
}
}
