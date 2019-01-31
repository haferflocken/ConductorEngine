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
	const float width,
	const float height,
	const float textHeight);
}
