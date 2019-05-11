#include <ecs/SerializedEntitiesAndComponents.h>

#include <mem/DeserializeLittleEndian.h>
#include <mem/SerializeLittleEndian.h>

#include <ostream>

void ECS::WriteSerializedEntitiesAndComponentsTo(
	const SerializedEntitiesAndComponents& serialization, std::ostream& fileOutput)
{
	WriteSerializedEntitiesAndComponentsTo(serialization,
		[&](const void* data, size_t length)
		{
			fileOutput.write(reinterpret_cast<const char*>(data), length);
		});
}

void ECS::WriteSerializedEntitiesAndComponentsTo(
	const SerializedEntitiesAndComponents& serialization,
	const std::function<void(const void*, size_t)>& outputFn)
{
	// Serialize the component views.
	Collection::Vector<uint8_t> viewBytes;

	const uint32_t numComponentTypes = serialization.m_componentViews.Size();
	Mem::LittleEndian::Serialize(numComponentTypes, viewBytes);

	for (const auto& entry : serialization.m_componentViews)
	{
		const char* const componentTypeName = Util::ReverseHash(entry.first.GetTypeHash());
		Mem::LittleEndian::Serialize(componentTypeName, viewBytes);

		const auto& componentViews = entry.second;

		const uint32_t numComponentViews = componentViews.Size();
		Mem::LittleEndian::Serialize(numComponentViews, viewBytes);

		const uint32_t componentViewsIndex = viewBytes.Size();
		const uint32_t sizeOfComponentViews = numComponentViews * sizeof(SerializedByteView);
		viewBytes.Resize(viewBytes.Size() + sizeOfComponentViews);
		memcpy(&viewBytes[componentViewsIndex], &componentViews.Front(), sizeOfComponentViews);
	}

	// Serialize the entity views.
	const uint32_t numEntityViews = serialization.m_entityViews.Size();
	Mem::LittleEndian::Serialize(numEntityViews, viewBytes);

	const uint32_t entityViewsIndex = viewBytes.Size();
	const uint32_t sizeOfEntityViews = numEntityViews * sizeof(SerializedByteView);
	viewBytes.Resize(viewBytes.Size() + sizeOfEntityViews);
	memcpy(&viewBytes[entityViewsIndex], &serialization.m_entityViews.Front(), sizeOfEntityViews);

	// Write the serialized views to the output.
	outputFn(&viewBytes.Front(), viewBytes.Size());

	// Write the serialized entities and components to the output.
	const uint32_t numBytes = serialization.m_bytes.Size();
	outputFn(&numBytes, sizeof(numBytes));
	outputFn(&serialization.m_bytes.Front(), serialization.m_bytes.Size());
}

bool ECS::TryReadSerializedEntitiesAndComponentsFrom(
	Collection::ArrayView<const uint8_t> fileBytes, SerializedEntitiesAndComponents& serialization)
{
	// Read the component views.
	const uint8_t* iter = fileBytes.begin();
	const uint8_t* const iterEnd = fileBytes.end();

	const auto maybeNumComponentTypes = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
	if (!maybeNumComponentTypes.second)
	{
		return false;
	}

	for (size_t i = 0; i < maybeNumComponentTypes.first; ++i)
	{
		char componentTypeNameBuffer[64];
		if (!Mem::LittleEndian::DeserializeString(iter, iterEnd, componentTypeNameBuffer))
		{
			return false;
		}
		const Util::StringHash componentTypeHash = Util::CalcHash(componentTypeNameBuffer);

		const auto maybeNumComponentViews = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
		if (!maybeNumComponentViews.second)
		{
			return false;
		}
		const uint32_t numComponentViews = maybeNumComponentViews.first;
		const uint32_t sizeOfComponentViews = numComponentViews * sizeof(SerializedByteView);
		if ((iterEnd - iter) < sizeOfComponentViews)
		{
			return false;
		}

		auto& componentViews = serialization.m_componentViews[ComponentType(componentTypeHash)];

		const uint32_t componentViewsIndex = componentViews.Size();
		componentViews.Resize(componentViews.Size() + numComponentViews);
		memcpy(&componentViews[componentViewsIndex], iter, sizeOfComponentViews);

		iter += sizeOfComponentViews;
	}

	// Read the entity views.
	const auto maybeNumEntityViews = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
	if (!maybeNumEntityViews.second)
	{
		return false;
	}
	const uint32_t numEntityViews = maybeNumEntityViews.first;
	const uint32_t sizeOfEntityViews = numEntityViews * sizeof(SerializedByteView);
	if ((iterEnd - iter) < sizeOfEntityViews)
	{
		return false;
	}

	const uint32_t entityViewsIndex = serialization.m_entityViews.Size();
	serialization.m_entityViews.Resize(serialization.m_entityViews.Size() + numEntityViews);
	memcpy(&serialization.m_entityViews[entityViewsIndex], iter, sizeOfEntityViews);

	iter += sizeOfEntityViews;

	// Read the serialized entities and components.
	const auto maybeNumBytes = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
	if (!maybeNumBytes.second)
	{
		return false;
	}
	const uint32_t numBytes = maybeNumBytes.first;
	if ((iterEnd - iter) < numBytes)
	{
		return false;
	}

	serialization.m_bytes.Resize(numBytes);
	memcpy(&serialization.m_bytes.Front(), iter, numBytes);

	iter += numBytes;

	return true;
}
