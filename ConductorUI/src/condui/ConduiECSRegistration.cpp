#include <condui/ConduiECSRegistration.h>

#include <condui/Condui.h>
#include <condui/RelativeUITransformSystem.h>
#include <condui/TextDisplayComponent.h>
#include <condui/TextInputComponent.h>
#include <condui/TextInputSystem.h>
#include <condui/UITransformComponent.h>

#include <ecs/ComponentInfoFactory.h>
#include <ecs/ComponentReflector.h>
#include <ecs/EntityInfoManager.h>
#include <ecs/EntityManager.h>

namespace Internal_ConduiEntityInfo
{
const Util::StringHash k_textDisplayInfoNameHash = Util::CalcHash("condui_text_display");
const Util::StringHash k_textInputInfoNameHash = Util::CalcHash("condui_text_input");
const Util::StringHash k_panelInfoNameHash = Util::CalcHash("condui_panel");
}

void Condui::RegisterComponentTypes(ECS::ComponentReflector& componentReflector,
	ECS::ComponentInfoFactory& componentInfoFactory)
{
	componentReflector.RegisterComponentType<TextDisplayComponent>();
	componentReflector.RegisterComponentType<TextInputComponent>();
	componentReflector.RegisterComponentType<UITransformComponent>();

	componentInfoFactory.RegisterFactoryFunction<TextDisplayComponentInfo>();
	componentInfoFactory.RegisterFactoryFunction<TextInputComponentInfo>();
	componentInfoFactory.RegisterFactoryFunction<UITransformComponentInfo>();
}

void Condui::RegisterSystems(ECS::EntityManager& entityManager, Input::CallbackRegistry& callbackRegistry)
{
	entityManager.RegisterSystem(Mem::MakeUnique<RelativeUITransformSystem>());
	entityManager.RegisterSystem(Mem::MakeUnique<TextInputSystem>(callbackRegistry));
}

void Condui::RegisterEntityInfo(ECS::EntityInfoManager& entityInfoManager,
	const uint16_t characterWidthPixels,
	const uint16_t characterHeightPixels,
	const File::Path& codePagePath)
{
	using namespace Internal_ConduiEntityInfo;
	{
		ECS::EntityInfo textDisplayInfo;
		textDisplayInfo.m_nameHash = k_textDisplayInfoNameHash;

		auto textDisplayComponentInfo = Mem::MakeUnique<TextDisplayComponentInfo>();
		textDisplayComponentInfo->m_characterWidthPixels = characterWidthPixels;
		textDisplayComponentInfo->m_characterHeightPixels = characterHeightPixels;
		textDisplayComponentInfo->m_codePagePath = codePagePath;

		textDisplayInfo.m_componentInfos.Add(std::move(textDisplayComponentInfo));
		textDisplayInfo.m_componentInfos.Add(Mem::MakeUnique<UITransformComponentInfo>());
		entityInfoManager.RegisterEntityInfo(k_textDisplayInfoNameHash, std::move(textDisplayInfo));
	}
	{
		ECS::EntityInfo textInputInfo;
		textInputInfo.m_nameHash = k_textInputInfoNameHash;

		auto textInputComponentInfo = Mem::MakeUnique<TextInputComponentInfo>();
		textInputComponentInfo->m_characterWidthPixels = characterWidthPixels;
		textInputComponentInfo->m_characterHeightPixels = characterHeightPixels;
		textInputComponentInfo->m_codePagePath = codePagePath;

		textInputInfo.m_componentInfos.Add(std::move(textInputComponentInfo));
		textInputInfo.m_componentInfos.Add(Mem::MakeUnique<UITransformComponentInfo>());
		entityInfoManager.RegisterEntityInfo(k_textInputInfoNameHash, std::move(textInputInfo));
	}
	{
		ECS::EntityInfo panelInfo;
		panelInfo.m_nameHash = k_panelInfoNameHash;
		panelInfo.m_componentInfos.Add(Mem::MakeUnique<UITransformComponentInfo>());
		entityInfoManager.RegisterEntityInfo(k_panelInfoNameHash, std::move(panelInfo));
	}
}

Util::StringHash Condui::GetEntityInfoNameHashFor(const ConduiElement& element)
{
	using namespace Internal_ConduiEntityInfo;

	Util::StringHash result;
	element.Match(
		[&](const TextDisplayElement&) { result = k_textDisplayInfoNameHash; },
		[&](const TextInputElement&) { result = k_textInputInfoNameHash; },
		[&](const PanelElement&) { result = k_panelInfoNameHash; });
	return result;
}
