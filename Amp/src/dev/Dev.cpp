#include <dev/Dev.h>

#include <iostream>

void Dev::PrintMessage(const MessageType messageType, const char* const message)
{
	std::cout << message << std::endl;
}
