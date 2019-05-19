#pragma once

#include <collection/Pair.h>
#include <cstdint>
#include <cstring>

namespace Mem::LittleEndian
{
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

inline Collection::Pair<uint16_t, bool> DeserializeUi16(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	if (bytes + 1 >= bytesEnd)
	{
		return { 0, false };
	}

	uint16_t out;
	memcpy(&out, bytes, sizeof(out));
	bytes += sizeof(out);

	return { out, true };
}

inline Collection::Pair<uint32_t, bool> DeserializeUi32(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	if (bytes + 3 >= bytesEnd)
	{
		return { 0, false };
	}

	uint32_t out;
	memcpy(&out, bytes, sizeof(out));
	bytes += sizeof(out);

	return { out, true };
}

inline Collection::Pair<uint64_t, bool> DeserializeUi64(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	if (bytes + 7 >= bytesEnd)
	{
		return { 0, false };
	}

	uint64_t out;
	memcpy(&out, bytes, sizeof(out));
	bytes += sizeof(out);

	return { out, true };
}

inline Collection::Pair<int8_t, bool> DeserializeI8(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	Collection::Pair<uint8_t, bool> out = DeserializeUi8(bytes, bytesEnd);
	return reinterpret_cast<Collection::Pair<int8_t, bool>&>(out);
}

inline Collection::Pair<int16_t, bool> DeserializeI16(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	Collection::Pair<uint16_t, bool> out = DeserializeUi16(bytes, bytesEnd);
	return reinterpret_cast<Collection::Pair<int16_t, bool>&>(out);
}

inline Collection::Pair<int32_t, bool> DeserializeI32(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	Collection::Pair<uint32_t, bool> out = DeserializeUi32(bytes, bytesEnd);
	return reinterpret_cast<Collection::Pair<int32_t, bool>&>(out);
}

inline Collection::Pair<int64_t, bool> DeserializeI64(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	Collection::Pair<uint64_t, bool> out = DeserializeUi64(bytes, bytesEnd);
	return reinterpret_cast<Collection::Pair<int64_t, bool>&>(out);
}

inline Collection::Pair<float, bool> DeserializeF32(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	if (bytes + 3 >= bytesEnd)
	{
		return { 0, false };
	}

	float out;
	memcpy(&out, bytes, sizeof(out));
	bytes += sizeof(out);

	return { out, true };
}

inline Collection::Pair<double, bool> DeserializeF64(const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	if (bytes + 7 >= bytesEnd)
	{
		return { 0, false };
	}

	double out;
	memcpy(&out, bytes, sizeof(out));
	bytes += sizeof(out);

	return { out, true };
}

template <size_t Capacity>
inline bool DeserializeString(const uint8_t*& bytes, const uint8_t* bytesEnd, char(&outStr)[Capacity])
{
	if (bytes + 2 >= bytesEnd)
	{
		return false;
	}

	const uint16_t length = DeserializeUi16(bytes, bytesEnd).first;

	if (bytes + length > bytesEnd || length >= Capacity)
	{
		return false;
	}

	memcpy(outStr, bytes, length);
	outStr[length] = '\0';
	bytes += length;

	return true;
}

template <size_t Capacity>
inline bool DeserializeString(const uint8_t*& bytes, const uint8_t* bytesEnd, wchar_t(&outStr)[Capacity])
{
	if (bytes + 2 >= bytesEnd)
	{
		return false;
	}

	const uint16_t length = DeserializeUi16(bytes, bytesEnd).first;

	if (bytes + (length * sizeof(wchar_t)) > bytesEnd || length >= Capacity)
	{
		return false;
	}

	memcpy(outStr, bytes, length * sizeof(wchar_t));
	outStr[length] = '\0';
	bytes += length;

	return true;
}
}
