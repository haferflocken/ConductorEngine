#include <behave/BehaviourTreeManager.h>
#include <behave/BehaviourTree.h>
#include <behave/parse/BehaviourTreeParser.h>

#include <dev/Dev.h>
#include <file/FullFileReader.h>

Behave::BehaviourTreeManager::BehaviourTreeManager(const BehaviourNodeFactory& nodeFactory)
	: m_nodeFactory(nodeFactory)
	, m_trees()
{
}

Behave::BehaviourTreeManager::~BehaviourTreeManager()
{
}

void Behave::BehaviourTreeManager::LoadTreesInDirectory(const File::Path& directory)
{
	if (!File::IsDirectory(directory))
	{
		Dev::LogWarning("Cannot load behaviour trees in \"%s\" because it is not a directory.", directory.c_str());
		return;
	}

	File::ForEachFileInDirectory(directory, [this](const File::Path& file) -> bool
		{
			if ((!file.has_extension())
				|| wcscmp(file.extension().c_str(), L".behave") != 0)
			{
				// Return true to keep iterating over the files.
				return true;
			}

			const std::string fileContents = File::ReadFullTextFile(file);
			const Parse::ParseResult parseResult = Parse::Parser::ParseTrees(fileContents.c_str());

			parseResult.Match(
				[&](const Collection::Vector<Parse::ParsedTree>& parsedTrees)
				{
					for (const auto& parsedTree : parsedTrees)
					{
						BehaviourTree tree;
						if (tree.LoadFromParsedTree(m_nodeFactory, parsedTree))
						{
							m_trees[Util::CalcHash(parsedTree.m_treeName)] = std::move(tree);
						}
						else
						{
							Dev::LogWarning("Failed to load behaviour tree \"%s\" from file [%S].",
								parsedTree.m_treeName.c_str(), file.c_str());
						}
					}
				},
				[&](const Parse::SyntaxError& syntaxError)
				{
					Dev::LogWarning("%s Ln %d Ch %d: %s", file.string().c_str(), syntaxError.m_lineNumber,
						syntaxError.m_characterInLine, syntaxError.m_message.c_str());
				});

			// Return true to keep iterating over the files.
			return true;
		});
}

const Behave::BehaviourTree* Behave::BehaviourTreeManager::FindTree(const Util::StringHash treeNameHash) const
{
	const auto itr = m_trees.Find(treeNameHash);
	return (itr != m_trees.end()) ? &itr->second : nullptr;
}
