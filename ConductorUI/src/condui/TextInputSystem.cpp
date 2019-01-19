#include <condui/TextInputSystem.h>

#include <ecs/EntityID.h>
#include <input/CallbackRegistry.h>
#include <input/InputMessage.h>
#include <math/Frustum.h>

namespace Condui
{
TextInputSystem::TextInputSystem(const Math::Frustum& sceneViewFrustum,
	Input::CallbackRegistry& inputCallbackRegistry)
	: SystemTempl()
	, m_sceneViewFrustum(sceneViewFrustum)
{
	using namespace Input;
	inputCallbackRegistry.RegisterInputCallback<InputMessage_MouseButtonDown>(
		[this](const InputMessage& message) { NotifyOfMouseDown(message); });
	inputCallbackRegistry.RegisterInputCallback<InputMessage_TextEditing>(
		[this](const InputMessage& message) { NotifyOfTextEditing(message); });
	inputCallbackRegistry.RegisterInputCallback<InputMessage_TextInput>(
		[this](const InputMessage& message) { NotifyOfTextInput(message); });
	inputCallbackRegistry.RegisterInputCallback<InputMessage_KeyDown>(
		[this](const InputMessage& message) { NotifyOfKeyDown(message); });
}

void TextInputSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	// If the mouse wasn't pressed last frame, the focus doesn't change.
	if (m_mouseDownScreenPos == Math::Vector2())
	{
		return;
	}

	// Determine which entity has text input focus, if any.
	const Math::Ray3 mouseRay = m_sceneViewFrustum.ProjectFromNearPlane(m_mouseDownScreenPos.x, m_mouseDownScreenPos.y);

	m_focusedComponent = nullptr;
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>();
		auto& textInputComponent = ecsGroup.Get<TextInputComponent>();
		
		const Math::Matrix4x4& modelToWorldMatrix = transformComponent.m_modelToWorldMatrix;
		const Math::Vector3& rootPosition = modelToWorldMatrix.GetTranslation();
		const Math::Vector3 scalePosition = modelToWorldMatrix * Math::Vector3(1.0f, 1.0f, 0.0f);

		// TODO(condui) determine if the ray intersects the rectangle from rootPosition to scalePosition

		// If the mouse is down within the entity, it gets focus.
		if (true)
		{
			m_focusedComponent = &textInputComponent;
			break;
		}
	}

	// Clear the mouse pressed state.
	m_mouseDownScreenPos = Math::Vector2();
}

void TextInputSystem::NotifyOfEntityAdded(const ECS::EntityID id, const ECSGroupType& group)
{
}

void TextInputSystem::NotifyOfEntityRemoved(const ECS::EntityID id, const ECSGroupType& group)
{
	if (m_focusedComponent == &group.Get<TextInputComponent>())
	{
		m_focusedComponent = nullptr;
	}
}

void TextInputSystem::NotifyOfMouseDown(const Input::InputMessage& message)
{
	auto& mouseDownMessage = message.Get<Input::InputMessage_MouseButtonDown>();
	m_mouseDownScreenPos.x = mouseDownMessage.m_mouseX;
	m_mouseDownScreenPos.y = mouseDownMessage.m_mouseY;
}

void TextInputSystem::NotifyOfTextEditing(const Input::InputMessage& message)
{
	if (m_focusedComponent == nullptr)
	{
		return;
	}
	auto& textEditingMessage = message.Get<Input::InputMessage_TextEditing>();
	// TODO(condui) do stuff in edit mode
}

void TextInputSystem::NotifyOfTextInput(const Input::InputMessage& message)
{
	if (m_focusedComponent == nullptr)
	{
		return;
	}
	auto& textInputMessage = message.Get<Input::InputMessage_TextInput>();
	m_focusedComponent->m_inputHandler(*m_focusedComponent, textInputMessage.m_text);
}

void TextInputSystem::NotifyOfKeyDown(const Input::InputMessage& message)
{
	if (m_focusedComponent == nullptr)
	{
		return;
	}
	auto& keyDownMessage = message.Get<Input::InputMessage_KeyDown>();
	switch (keyDownMessage.m_keyCode)
	{
	case '\b':
	{
		m_focusedComponent->m_inputHandler(*m_focusedComponent, "\b");
		break;
	}
	case '\r':
	{
		m_focusedComponent->m_inputHandler(*m_focusedComponent, "\r");
		break;
	}
	case '\t':
	{
		m_focusedComponent->m_inputHandler(*m_focusedComponent, "\t");
		break;
	}
	}
}
}
