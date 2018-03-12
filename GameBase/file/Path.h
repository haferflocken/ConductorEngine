#pragma once

#include <dev/Dev.h>

#include <filesystem>
#include <functional>

namespace File
{
using Path = std::experimental::filesystem::v1::path;

inline Path MakePath(const char* pathString) { return std::experimental::filesystem::v1::u8path(pathString); }

inline bool IsDirectory(const Path& path) { return std::experimental::filesystem::v1::is_directory(path); }
inline bool IsRegularFile(const Path& path) { return std::experimental::filesystem::v1::is_regular_file(path); }

inline void ForEachFileInDirectory(const Path& directory, const std::function<bool(const Path&)>& fn)
{
	Dev::Assert(IsDirectory(directory), "\"%s\" is not a directory.", directory.c_str());

	using Iterator = std::experimental::filesystem::v1::directory_iterator;
	for (auto i = Iterator(directory), iEnd = Iterator(); i != iEnd; ++i)
	{
		const Path& element = i->path();

		if (IsRegularFile(element) && !fn(element))
		{
			break;
		}
	}
}
}
