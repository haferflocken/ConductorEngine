// GENERATED CODE
#pragma once

#include <ecs/ActorComponent.h>
#include <collection/Vector.h>
#include <cstdint>
#include <string>

namespace ECS { class ActorComponentVector; }

namespace Behave::Components
{
class ComponentTestInfo;

/**
 * Component Test
 */
class ComponentTest final : public ECS::ActorComponent
{
public:
	using Info = ComponentTestInfo;
	static bool TryCreateFromInfo(const Info& componentInfo, 
		const ActorComponentID reservedID, ActorComponentVector& destination);
	
	explicit ComponentTest(const ECS::ActorComponentID id)
		: ECS::ActorComponent(id)
	{}
	
	ComponentTest(const ComponentTest&) = delete;
	ComponentTest& operator=(const ComponentTest&) = delete;
	
	ComponentTest(ComponentTest&&) = default;
	ComponentTest& operator=(ComponentTest&&) = default;
	
	virtual ~ComponentTest() {}
	
	// Description of Foo
	bool m_foo{ true };
	struct Bar
	{
		struct Car
		{
			// Description of numWheels
			int32_t m_numWheels{ 4 };
			float m_topSpeed{ 0.000000 };
		};
		Collection::Vector<Collection::Vector<Car>> m_baz;
	};
	Bar m_bar;
};
}
