#pragma once

namespace Host
{
// IHost is the interface a game's host must implement.
class IHost
{
public:
	virtual void Update() = 0;
};
}
