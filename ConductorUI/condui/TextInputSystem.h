#pragma once

#include <condui/TextInputComponent.h>
#include <ecs/System.h>
#include <math/Vector2.h>
#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>

namespace ECS { class EntityID; }

namespace Input
{
class CallbackRegistry;
struct InputMessage;
}

namespace Condui
{
/**
 * The TextInputSystem manages the state of entities with TextInputComponents. At any given time, zero or one entities
 * may have text input focus. Entities gain focus by being clicked on. The entity in focus receives all text input so
 * long as it's in focus.
 */
class TextInputSystem final : public ECS::SystemTempl<
	Util::TypeList<Scene::SceneTransformComponent>,
	Util::TypeList<TextInputComponent>,
	ECS::SystemBindingType::Extended>
{
public:
	TextInputSystem(Input::CallbackRegistry& inputCallbackRegistry);
	virtual ~TextInputSystem() {}

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

	void NotifyOfEntityAdded(const ECS::EntityID id, const ECSGroupType& group);
	void NotifyOfEntityRemoved(const ECS::EntityID id, const ECSGroupType& group);

private:
	void NotifyOfMouseDown(const Input::InputMessage& message);
	void NotifyOfTextEditing(const Input::InputMessage& message);
	void NotifyOfTextInput(const Input::InputMessage& message);
	void NotifyOfKeyDown(const Input::InputMessage& message);

	// The position where the mouse was pressed. Is Vector2() when there wasn't a mouse press last frame.
	Math::Vector2 m_mouseDownPos{};

	// The TextInputComponent of the entity that is currently receiving input.
	TextInputComponent* m_focusedComponent{ nullptr };
};
}
