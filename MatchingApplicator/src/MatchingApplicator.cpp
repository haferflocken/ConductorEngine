#include <regex>
#include <string>

/**
 * Given a string, runs a command if the pattern matches the string.
 * The input string is specified with --input.
 * The pattern is specified with --pattern REGEX
 * The command is specified with --command COMMAND
 */
int main(const int argc, const char* argv[])
{
	// Parse arguments.
	if (argc <= 1)
	{
		return 1;
	}

	const char* inputString = nullptr;
	const char* patternString = nullptr;
	std::regex pattern;
	std::string command;

	for (int i = 0; i < argc; ++i)
	{
		const char* const arg = argv[i];
		if (strcmp(arg, "--pattern") == 0)
		{
			if ((i + 1) < argc)
			{
				try
				{
					patternString = argv[i + 1];
					pattern = std::regex(patternString);
					i += 1;
				}
				catch (const std::regex_error& e)
				{
					printf("Failed to parse regex pattern \"%s\": %s\n", argv[i + 1], e.what());
					return false;
				}
			}
			else
			{
				printf("--pattern requires a regex pattern following it.\n");
				return 1;
			}
		}
		else if (strcmp(arg, "--command") == 0)
		{
			if ((i + 1) >= argc)
			{
				printf("--command requires a command following it.\n");
				return 1;
			}
			if (!command.empty())
			{
				printf("Multiple commands may not be specified!\n");
				return 1;
			}

			for (++i; i < argc; ++i)
			{
				const char* str = argv[i];
				if (strcmp(str, "--input") == 0 || strcmp(str, "--pattern") == 0 || strcmp(str, "--command") == 0)
				{
					--i;
					break;
				}
				command.append(" ");
				command.append(str);
			}
		}
		else if (strcmp(arg, "--input") == 0)
		{
			if (inputString == nullptr && (i + 1) < argc)
			{
				inputString = argv[i + 1];
				i += 1;
			}
			else if (inputString == nullptr)
			{
				printf("--input requires a string following it.\n");
				return 1;
			}
			else
			{
				printf("Multiple input strings may not be specified!\n");
				return 1;
			}
		}
	}

	if (inputString == nullptr)
	{
		printf("An input string must be specified using --input\n");
		return 1;
	}
	if (command.empty())
	{
		printf("A command must be specified using --command\n");
		return 1;
	}

	// Attempt to match the pattern and execute the command.
	if (std::regex_match(inputString, pattern))
	{
		printf("\"%s\" matches pattern \"%s\". Executing command: %s\n", inputString, patternString, command.c_str());
		const int commandResult = system(command.c_str());
		return commandResult;
	}
	return 0;
}
