#pragma once

#include <collection/Vector.h>
#include <cstdint>

namespace Mem::LittleEndian
{
inline void Serialize(uint8_t v, Collection::Vector<uint8_t>& out)
{
	out.Add(v);
}

inline void Serialize(uint16_t v, Collection::Vector<uint8_t>& out)
{
	const size_t i = out.Size();
	out.Resize(out.Size() + sizeof(v));
	memcpy(out.begin() + i, &v, sizeof(v));
}

inline void Serialize(uint32_t v, Collection::Vector<uint8_t>& out)
{
	const size_t i = out.Size();
	out.Resize(out.Size() + sizeof(v));
	memcpy(out.begin() + i, &v, sizeof(v));
}

inline void Serialize(uint64_t v, Collection::Vector<uint8_t>& out)
{
	const size_t i = out.Size();
	out.Resize(out.Size() + sizeof(v));
	memcpy(out.begin() + i, &v, sizeof(v));
}

inline void Serialize(int8_t v, Collection::Vector<uint8_t>& out)
{
	Serialize(static_cast<uint8_t>(v), out);
}

inline void Serialize(int16_t v, Collection::Vector<uint8_t>& out)
{
	Serialize(static_cast<uint16_t>(v), out);
}

inline void Serialize(int32_t v, Collection::Vector<uint8_t>& out)
{
	Serialize(static_cast<uint32_t>(v), out);
}

inline void Serialize(int64_t v, Collection::Vector<uint8_t>& out)
{
	Serialize(static_cast<uint64_t>(v), out);
}

inline void Serialize(const char* str, Collection::Vector<uint8_t>& out)
{
	uint16_t length = 0;
	while (str[length] != '\0')
	{
		++length;
	}

	Serialize(length, out);
	out.AddAll({ reinterpret_cast<const uint8_t*>(str), length });
}
}
