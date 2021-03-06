#pragma once

#include <dev/Dev.h>

#include <filesystem>
#include <functional>

namespace File
{
namespace FileSystem = std::experimental::filesystem::v1;

using Path = FileSystem::path;

inline Path MakePath(const char* pathString) { return FileSystem::u8path(pathString); }
inline Path MakePath(const wchar_t* pathString) { return Path{ pathString }; }

inline bool Exists(const Path& path) { return FileSystem::exists(path); }
inline bool IsDirectory(const Path& path) { return FileSystem::is_directory(path); }
inline bool IsRegularFile(const Path& path) { return FileSystem::is_regular_file(path); }

inline void ForEachFileInDirectory(const Path& directory, const std::function<bool(const Path&)>& fn)
{
	AMP_ASSERT(IsDirectory(directory), "\"%s\" is not a directory.", directory.u8string().c_str());

	using Iterator = FileSystem::directory_iterator;
	for (auto i = Iterator(directory), iEnd = Iterator(); i != iEnd; ++i)
	{
		const Path& element = i->path();

		if (IsRegularFile(element) && !fn(element))
		{
			break;
		}
	}
}

inline bool ForEachFileInDirectoryRecursive(
	const Path& directory,
	const std::function<bool(const Path&, size_t)>& fn,
	const size_t depth = 0)
{
	AMP_ASSERT(IsDirectory(directory), "\"%s\" is not a directory.", directory.u8string().c_str());

	using Iterator = FileSystem::directory_iterator;
	for (auto i = Iterator(directory), iEnd = Iterator(); i != iEnd; ++i)
	{
		const Path& element = i->path();

		if (IsDirectory(element) && !ForEachFileInDirectoryRecursive(element, fn, depth + 1))
		{
			return false;
		}
		if (IsRegularFile(element) && !fn(element, depth))
		{
			return false;
		}
	}
	return true;
}
}
