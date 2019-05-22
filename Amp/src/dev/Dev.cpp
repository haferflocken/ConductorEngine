#include <dev/Dev.h>

#include <iostream>

namespace Internal_Dev
{
std::ostream* g_outputByMessageType[static_cast<size_t>(Dev::MessageType::Count)] = {
	&std::cout,
	&std::cout,
	&std::clog,
	&std::clog
};
}

void Dev::SetOutputFor(const MessageType messageType, std::ostream& ostream)
{
	using namespace Internal_Dev;
	g_outputByMessageType[static_cast<size_t>(messageType)] = &ostream;
}

void Dev::PrintMessage(const MessageType messageType, const char* const message)
{
	using namespace Internal_Dev;

	const size_t i = static_cast<size_t>(messageType);
	if (i >= static_cast<size_t>(MessageType::Count))
	{
		std::cerr << "INVALID MESSAGE TYPE " << i << " ENCOUNTERED" << std::endl;
		__debugbreak();
		return;
	}

	*g_outputByMessageType[i] << message << std::endl;
}
