// GENERATED CODE
#include <behave/ActorComponent.h>
#include <collection/Vector.h>
#include <cstdint>
#include <string>

namespace Behave { class ActorComponentVector; }

namespace Behave::Components
{
class ComponentTestInfo;

class ComponentTest final : public Behave::ActorComponent
{
public:
	using Info = ComponentTestInfo;
	static bool TryCreateFromInfo(const Info& componentInfo, 
		const ActorComponentID reservedID, ActorComponentVector& destination);
	
	explicit ComponentTest(const Behave::ActorComponentID id)
		: Behave::ActorComponent(id)
	{}
	
	ComponentTest(const ComponentTest&) = delete;
	ComponentTest& operator=(const ComponentTest&) = delete;
	
	ComponentTest(ComponentTest&&) = default;
	ComponentTest& operator=(ComponentTest&&) = default;
	
	virtual ~ComponentTest() {}
	
	bool m_foo{ true };
	struct Bar
	{
		struct Car
		{
			int32_t m_numWheels{ 4 };
			float m_topSpeed{ 0.000000 };
		};
		Collection::Vector<Collection::Vector<Car>> m_baz{  };
	};
	Bar m_bar;
};
}
