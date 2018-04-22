#pragma once

namespace Conductor
{
enum class ApplicationErrorCode : int
{
	NoError = 0,
	MissingDatapath,
	MissingApplicationMode,
	MissingClientHostName,
	MissingClientHostPort,
	MissingHostPort,
	FailedToInitializeSocketAPI,
	FailedToInitializeNetworkThread,
};
}
