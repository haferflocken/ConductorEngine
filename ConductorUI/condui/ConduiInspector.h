#pragma once

#include <condui/Condui.h>

namespace Mem
{
struct InspectorInfo;
}

namespace Condui
{
ConduiElement MakeInspectorElement(
	const Mem::InspectorInfo* const inspectorInfo,
	void* const subject,
	const float xScale,
	const float yScale,
	const float textVerticalScale);
}
