#pragma once

#include <cstdint>

namespace Collection
{
template <typename T> class ArrayView;
template <typename T> class Vector;
}

namespace Network::DeltaCompression
{
void Compress(
	const Collection::ArrayView<const uint8_t>& lastSeenBytes,
	const Collection::ArrayView<const uint8_t>& currentBytes,
	Collection::Vector<uint8_t>& outCompressedBytes);

bool TryDecompress(
	const Collection::ArrayView<const uint8_t>& lastSeenBytes,
	const Collection::ArrayView<const uint8_t>& compressedBytes,
	Collection::Vector<uint8_t>& outDecompressedBytes);
}
