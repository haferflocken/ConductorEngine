#include <host/IHost.h>

Collection::Vector<uint8_t> Host::IHost::SerializeECSUpdateTransmission()
{
	return m_entityManager.SerializeDeltaTransmission();
}
