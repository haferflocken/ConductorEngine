#include <util/StringHash.h>

Mem::UniquePtr<Util::StringHash::Dictionary> Util::StringHash::s_dictionary;

Util::StringHash::Dictionary& Util::StringHash::GetDictionary()
{
	if (s_dictionary == nullptr)
	{
		s_dictionary = Mem::MakeUnique<Dictionary>();
	}
	return *s_dictionary;
}

Util::StringHash Util::CalcHash(const char* const cStr)
{
	auto str = std::string(cStr);
	const auto hash = StringHash(std::hash<std::string>()(str));

	Util::StringHash::Dictionary& dictionary = Util::StringHash::GetDictionary();
	const auto itr = dictionary.find(hash);
	if (itr == dictionary.end())
	{
		dictionary[hash] = std::move(str);
	}

	return hash;
}

Util::StringHash Util::CalcHash(const std::string& str)
{
	const auto hash = StringHash(std::hash<std::string>()(str));
	
	Util::StringHash::Dictionary& dictionary = Util::StringHash::GetDictionary();
	const auto itr = dictionary.find(hash);
	if (itr == dictionary.end())
	{
		dictionary[hash] = str;
	}

	return hash;
}

const char* Util::ReverseHash(const Util::StringHash hash)
{
	const Util::StringHash::Dictionary& dictionary = Util::StringHash::GetDictionary();
	const auto itr = dictionary.find(hash);
	return (itr != dictionary.end()) ? itr->second.c_str() : "\0";
}
