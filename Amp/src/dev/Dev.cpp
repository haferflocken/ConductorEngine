#include <dev/Dev.h>

#include <iostream>

std::ostream* Dev::s_outputByMessageType[] = {
	&std::cout,
	&std::cout,
	&std::clog,
	&std::clog
};

void Dev::SetOutputFor(const MessageType messageType, std::ostream& ostream)
{
	s_outputByMessageType[static_cast<size_t>(messageType)] = &ostream;
}

void Dev::PrintMessage(const MessageType messageType, const char* const message)
{
	const size_t i = static_cast<size_t>(messageType);
	if (i >= static_cast<size_t>(MessageType::Count))
	{
		std::cerr << "INVALID MESSAGE TYPE " << i << " ENCOUNTERED" << std::endl;
		__debugbreak();
		return;
	}

	*s_outputByMessageType[i] << message << std::endl;
}
