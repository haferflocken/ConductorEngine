#pragma once

#include <unordered_map>

namespace Collection
{
/**
 * Simplifies extraction of command line arguments by storing them in a dictionary and providing a convenient interface
 * for querying them.
 */
class ProgramParameters
{
public:
	ProgramParameters(const int argCount, const char* argVals[])
		: m_data()
	{
		// Extract parameters which start with '-' as keys and the parameter following those as their value.
		for (int i = 0; i < argCount; ++i)
		{
			if (argVals[i][0] == '-')
			{
				if ((i + 1) < argCount && argVals[i + 1][0] != '-')
				{
					// Store the key/value pair and skip the next parameter.
					m_data[argVals[i]] = argVals[i + 1];
					++i; 
				}
				else
				{
					// Store the parameter without an associated value.
					m_data[argVals[i]] = "";
				}
			}
		}
	}

	bool TryGet(const char* const key, std::string& outVal) const
	{
		const auto itr = m_data.find(key);
		if (itr != m_data.end())
		{
			outVal = itr->second;
			return true;
		}
		return false;
	}

private:
	std::unordered_map<std::string, std::string> m_data;
};
}
