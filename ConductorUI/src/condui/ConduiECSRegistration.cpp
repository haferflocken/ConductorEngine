#include <condui/ConduiECSRegistration.h>

#include <condui/Condui.h>
#include <condui/RelativeUITransformSystem.h>
#include <condui/TextDisplayComponent.h>
#include <condui/UITransformComponent.h>

#include <ecs/ComponentInfoFactory.h>
#include <ecs/ComponentReflector.h>
#include <ecs/EntityInfoManager.h>
#include <ecs/EntityManager.h>

namespace Internal_ConduiEntityInfo
{
const Util::StringHash k_textDisplayInfoNameHash = Util::CalcHash("condui_text_display");
const Util::StringHash k_panelInfoNameHash = Util::CalcHash("condui_panel");
}

void Condui::RegisterComponentTypes(ECS::ComponentReflector& componentReflector,
	ECS::ComponentInfoFactory& componentInfoFactory)
{
	componentReflector.RegisterComponentType<TextDisplayComponent>();
	componentReflector.RegisterComponentType<UITransformComponent>();

	componentInfoFactory.RegisterFactoryFunction<TextDisplayComponentInfo>();
	componentInfoFactory.RegisterFactoryFunction<UITransformComponentInfo>();
}

void Condui::RegisterSystems(ECS::EntityManager& entityManager)
{
	entityManager.RegisterSystem(Mem::MakeUnique<RelativeUITransformSystem>());
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
		[&](const PanelElement&) { result = k_panelInfoNameHash; });
	return result;
}
