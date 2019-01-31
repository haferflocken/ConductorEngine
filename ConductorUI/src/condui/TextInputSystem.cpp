#include <condui/TextInputSystem.h>

#include <ecs/EntityID.h>
#include <input/CallbackRegistry.h>
#include <input/InputMessage.h>
#include <math/Frustum.h>

namespace Condui
{
namespace Internal_TextInputSystem
{
// The vertices of the rectangle must be in counterclockwise order, they must form 4 right angles,
// and the given point must be on the rectangle for this to work.
bool IsPointWithinRectangle(const Math::Vector3& rectV0,
	const Math::Vector3& rectV1,
	const Math::Vector3& rectV2,
	const Math::Vector3& rectV3,
	const Math::Vector3& point)
{
	const Math::Vector3 v0ToPoint = point - rectV0;
	const Math::Vector3 v0ToV1 = rectV1 - rectV0;
	if (v0ToPoint.Dot(v0ToV1) < 0.0f)
	{
		return false;
	}
	const Math::Vector3 v1ToPoint = point - rectV1;
	const Math::Vector3 v1ToV2 = rectV2 - rectV1;
	if (v1ToPoint.Dot(v1ToV2) < 0.0f)
	{
		return false;
	}
	const Math::Vector3 v2ToPoint = point - rectV2;
	const Math::Vector3 v2ToV3 = rectV3 - rectV2;
	if (v2ToPoint.Dot(v2ToV3) < 0.0f)
	{
		return false;
	}
	const Math::Vector3 v3ToPoint = point - rectV3;
	const Math::Vector3 v3ToV0 = rectV0 - rectV3;
	if (v3ToPoint.Dot(v3ToV0) < 0.0f)
	{
		return false;
	}
	return true;
}
}

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
	using namespace Internal_TextInputSystem;

	// If the mouse wasn't pressed last frame, the focus doesn't change.
	if (m_mouseDownScreenPos == Math::Vector2())
	{
		return;
	}

	// Determine which entity has text input focus, if any.
	const Math::Ray3 mouseRay = m_sceneViewFrustum.ProjectThroughNearPlane(m_mouseDownScreenPos.x, m_mouseDownScreenPos.y);

	m_focusedComponent = nullptr;
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>();
		auto& textInputComponent = ecsGroup.Get<TextInputComponent>();

		const Math::Matrix4x4& modelToWorldMatrix = transformComponent.m_modelToWorldMatrix;

		const Math::Vector3& a = modelToWorldMatrix.GetTranslation();
		const Math::Vector3 b = modelToWorldMatrix * Math::Vector3(textInputComponent.m_width, 0.0f, 0.0f);
		const Math::Vector3 c = modelToWorldMatrix * Math::Vector3(textInputComponent.m_width, textInputComponent.m_height, 0.0f);
		const Math::Vector3 d = modelToWorldMatrix * Math::Vector3(0.0f, textInputComponent.m_height, 0.0f);

		const Math::Vector3 unscaledNormal = (b - a).Cross(c - a);
		const Math::Vector3 normal = unscaledNormal / unscaledNormal.Length();

		Math::Vector3 p;
		if (!mouseRay.TryCalcIntersectionWithPlane(normal, a, p))
		{
			continue;
		}

		// If the mouse is down within the entity, it gets focus.
		if (IsPointWithinRectangle(a, b, c, d, p))
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
