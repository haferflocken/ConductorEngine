#pragma once

#include <collection/Variant.h>
#include <string>

namespace ECS
{
struct ApplyDeltaTransmission_Success {};
struct ApplyDeltaTransmission_DataTooShort {};
struct ApplyDeltaTransmission_UnrecognizedComponentType
{
	explicit ApplyDeltaTransmission_UnrecognizedComponentType(const char* typeName)
		: m_unrecognizedType(typeName)
	{}

	std::string m_unrecognizedType;
};
struct ApplyDeltaTransmission_ComponentAddedOutOfOrder {};
struct ApplyDeltaTransmission_EntityAddedOutOfOrder {};

using ApplyDeltaTransmissionResult = Collection::Variant<
	ApplyDeltaTransmission_Success,
	ApplyDeltaTransmission_DataTooShort,
	ApplyDeltaTransmission_UnrecognizedComponentType,
	ApplyDeltaTransmission_ComponentAddedOutOfOrder,
	ApplyDeltaTransmission_EntityAddedOutOfOrder>;
}
