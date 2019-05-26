#pragma once

namespace ECS
{
class Entity;
class EntityManager;
}

namespace Condui
{
struct FontInfo;
}

namespace ProfilerUI
{
// Creates an entity that draws Amp's profiling data.
ECS::Entity& CreateProfilerEntity(ECS::EntityManager& entityManager,
	const float width, const float height, const float textHeight, const Condui::FontInfo* font);
}
