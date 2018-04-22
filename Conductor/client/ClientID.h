#pragma once

#include <unit/UnitTempl.h>

#include <cstdint>

namespace Client
{
struct ClientID : public Unit::UnitTempl<ClientID, uint16_t>
{
	constexpr ClientID()
		: BaseType(0)
	{}

	explicit constexpr ClientID(BackingType n)
		: BaseType(n)
	{}

	bool IsValid() const { return m_n != 0; }
};
}
