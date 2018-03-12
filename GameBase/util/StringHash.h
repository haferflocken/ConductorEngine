#pragma once

#include <mem/UniquePtr.h>

#include <functional>
#include <string>
#include <unordered_map>

namespace Util
{
class StringHash
{
public:
	using Dictionary = std::unordered_map<StringHash, std::string>;
	
	static Dictionary& GetDictionary();
	static constexpr size_t sk_invalidHash = 0;

	constexpr StringHash()
		: m_hash(sk_invalidHash)
	{}

	explicit constexpr StringHash(size_t hash)
		: m_hash(hash)
	{}

	StringHash(const StringHash& other) = default;
	StringHash& operator=(const StringHash& rhs) = default;

	size_t Get() const { return m_hash; }

	bool operator==(const StringHash& rhs) const { return m_hash == rhs.m_hash; }
	bool operator!=(const StringHash& rhs) const { return m_hash != rhs.m_hash; }
	bool operator<=(const StringHash& rhs) const { return m_hash <= rhs.m_hash; }
	bool operator>=(const StringHash& rhs) const { return m_hash >= rhs.m_hash; }
	bool operator<(const StringHash& rhs) const { return m_hash < rhs.m_hash; }
	bool operator>(const StringHash& rhs) const { return m_hash > rhs.m_hash; }
	
private:
	static Mem::UniquePtr<Dictionary> s_dictionary;

	size_t m_hash;
};

StringHash CalcHash(const char* const cStr);
StringHash CalcHash(const std::string& str);
const char* ReverseHash(const StringHash hash);
}

namespace Traits
{
template <typename T>
struct IsMemCopyAFullCopy;

template <> struct IsMemCopyAFullCopy<Util::StringHash> : std::true_type {};
}

namespace std
{
template <>
struct hash<Util::StringHash>
{
	size_t operator()(const Util::StringHash& value) const
	{
		return std::hash<size_t>()(value.Get());
	}
};
}
