#include <file/FullFileReader.h>

#include <fstream>
#include <sstream>

std::string File::ReadFullTextFile(const Path& path)
{
	std::ifstream in = std::ifstream(path.c_str(), std::ios::binary);
	return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}
