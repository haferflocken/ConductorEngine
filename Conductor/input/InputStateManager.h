#pragma once

#include <collection/VectorMap.h>
#include <input/InputSource.h>
#include <input/InputStateBuffer.h>
#include <util/StringHash.h>

namespace Input
{
class CallbackRegistry;
struct InputMessage;

/**
 * The InputStateManager converts InputMessages to named input states.
 * This satisfies the dual needs of tracking persistent input state and control mapping.
 */
class InputStateManager final
{
public:
	InputStateManager();
	explicit InputStateManager(Input::CallbackRegistry& callbackRegistry);

	void SetInputName(const InputSource source, const Util::StringHash nameHash);

	const InputStateBuffer* FindInput(const InputSource inputSource) const;
	const InputStateBuffer* FindNamedInput(const Util::StringHash nameHash) const;

	void ResetInputStates();
	
	Collection::Vector<uint8_t> SerializeFullTransmission() const;
	void ApplyFullTransmission(const Collection::Vector<uint8_t>& transmissionBytes);

private:
	void NotifyOfInputMessage(const InputMessage& message);

	Collection::VectorMap<Util::StringHash, InputStateBuffer> m_namedInputStateBuffers;
	Collection::VectorMap<InputSource, Util::StringHash> m_namedInputMapping;
};
}
