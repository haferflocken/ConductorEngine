// GENERATED CODE
#include <collection/Vector.h>
#include <cstdint>
#include <string>

namespace InfoAsset
{
struct Test
{
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
	Bar bar;
};
}
// GENERATED CODE
#include <json/JSONTypes.h>

namespace InfoAsset
{
JSON::JSONObject SaveInfoInstance(const Test& infoInstance)
{
	return JSON::JSONObject();
}
}
