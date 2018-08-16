#pragma once

#include <collection/Pair.h>
#include <collection/Vector.h>
#include <cstdint>

namespace Mem
{
inline void Serialize(uint8_t v, Collection::Vector<uint8_t>& out)
{
	out.Add(v);
}

inline Collection::Pair<uint8_t, bool> DeserializeUi8(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	if (bytes >= bytesEnd)
	{
		return { 0, false };
	}

	const uint8_t out = *bytes;
	++bytes;
	return { out, true };
}

inline void Serialize(uint16_t v, Collection::Vector<uint8_t>& out)
{
	out.Add(static_cast<uint8_t>(v >> 8));
	out.Add(static_cast<uint8_t>(v));
}

inline Collection::Pair<uint16_t, bool> DeserializeUi16(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	if (bytes + 1 >= bytesEnd)
	{
		return { 0, false };
	}

	uint16_t out = *(bytes++);
	out <<= 8;
	out |= *(bytes++);
	return { out, true };
}

inline void Serialize(uint32_t v, Collection::Vector<uint8_t>& out)
{
	out.Add(static_cast<uint8_t>(v >> 24));
	out.Add(static_cast<uint8_t>(v >> 16));
	out.Add(static_cast<uint8_t>(v >> 8));
	out.Add(static_cast<uint8_t>(v));
}

inline Collection::Pair<uint32_t, bool> DeserializeUi32(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	if (bytes + 3 >= bytesEnd)
	{
		return { 0, false };
	}

	uint32_t out = *(bytes++);
	out <<= 8;
	out |= *(bytes++);
	out <<= 8;
	out |= *(bytes++);
	out <<= 8;
	out |= *(bytes++);

	return { out, true };
}

inline void Serialize(uint64_t v, Collection::Vector<uint8_t>& out)
{
	out.Add(static_cast<uint8_t>(v >> 56));
	out.Add(static_cast<uint8_t>(v >> 48));
	out.Add(static_cast<uint8_t>(v >> 40));
	out.Add(static_cast<uint8_t>(v >> 32));
	out.Add(static_cast<uint8_t>(v >> 24));
	out.Add(static_cast<uint8_t>(v >> 16));
	out.Add(static_cast<uint8_t>(v >> 8));
	out.Add(static_cast<uint8_t>(v));
}

inline Collection::Pair<uint64_t, bool> DeserializeUi64(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	if (bytes + 7 >= bytesEnd)
	{
		return { 0, false };
	}

	uint64_t out = *(bytes++);
	out <<= 8;
	out |= *(bytes++);
	out <<= 8;
	out |= *(bytes++);
	out <<= 8;
	out |= *(bytes++);
	out <<= 8;
	out |= *(bytes++);
	out <<= 8;
	out |= *(bytes++);
	out <<= 8;
	out |= *(bytes++);
	out <<= 8;
	out |= *(bytes++);

	return { out, true };
}

inline void Serialize(int8_t v, Collection::Vector<uint8_t>& out)
{
	Serialize(static_cast<uint8_t>(v), out);
}

inline Collection::Pair<int8_t, bool> DeserializeI8(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	Collection::Pair<uint8_t, bool> out = DeserializeUi8(bytes, bytesEnd);
	return reinterpret_cast<Collection::Pair<int8_t, bool>&>(out);
}

inline void Serialize(int16_t v, Collection::Vector<uint8_t>& out)
{
	Serialize(static_cast<uint16_t>(v), out);
}

inline Collection::Pair<int16_t, bool> DeserializeI16(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	Collection::Pair<uint16_t, bool> out = DeserializeUi16(bytes, bytesEnd);
	return reinterpret_cast<Collection::Pair<int16_t, bool>&>(out);
}

inline void Serialize(int32_t v, Collection::Vector<uint8_t>& out)
{
	Serialize(static_cast<uint32_t>(v), out);
}

inline Collection::Pair<int32_t, bool> DeserializeI32(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	Collection::Pair<uint32_t, bool> out = DeserializeUi32(bytes, bytesEnd);
	return reinterpret_cast<Collection::Pair<int32_t, bool>&>(out);
}

inline void Serialize(int64_t v, Collection::Vector<uint8_t>& out)
{
	Serialize(static_cast<uint64_t>(v), out);
}

inline Collection::Pair<int64_t, bool> DeserializeI64(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	Collection::Pair<uint64_t, bool> out = DeserializeUi64(bytes, bytesEnd);
	return reinterpret_cast<Collection::Pair<int64_t, bool>&>(out);
}

inline void Serialize(const char* str, Collection::Vector<uint8_t>& out)
{
	while (*str != '\0')
	{
		out.Add(*str);
		++str;
	}
	out.Add('\0');
}

template <size_t Capacity>
inline bool DeserializeString(const uint8_t*& bytes, const uint8_t* bytesEnd, char (&outStr)[Capacity])
{
	if (bytes + Capacity - 1 >= bytesEnd)
	{
		return false;
	}

	size_t i = 0;
	while (*bytes != '\0' && i < Capacity - 1)
	{
		outStr[i++] = *(bytes++);
	}

	if (i == (Capacity - 1) && *bytes != '\0')
	{
		return false;
	}

	outStr[i] = '\0';
	return true;
}
}
