#pragma once

namespace Conductor
{
enum class ApplicationErrorCode : int
{
	NoError = 0,
	MissingDataPath,
	FailedToGetUserPath,
	MissingApplicationMode,
	MissingClientHostName,
	MissingClientHostPort,
	MissingHostPort,
	FailedToInitializeSocketAPI,
	FailedToInitializeNetworkThread,
};
}
