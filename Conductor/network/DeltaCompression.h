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

// Decompresses into inOutDecompressedBytes without writing past the end of inOutDecompressedBytes.
// Shrinks inOutDecompressedBytes to the size of the decompressed data.
bool TryDecompress(
	const Collection::ArrayView<const uint8_t>& lastSeenBytes,
	const uint8_t*& compressedBytesIter,
	const uint8_t* const compressedBytesEnd,
	Collection::ArrayView<uint8_t>& inOutDecompressedBytes);
}
