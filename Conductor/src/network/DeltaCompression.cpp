#include <network/DeltaCompression.h>

#include <collection/ArrayView.h>
#include <collection/Vector.h>
#include <mem/DeserializeLittleEndian.h>
#include <mem/SerializeLittleEndian.h>

namespace Network
{
namespace Internal_DeltaCompression
{
constexpr uint16_t k_identicalBytesMarker = UINT16_MAX;

constexpr uint8_t k_unchangedSectionTypeID = 0x0F;
constexpr uint8_t k_changedSectionTypeID = 0xF0;
constexpr uint8_t k_trailingSectionTypeID = 0xAA;
constexpr uint8_t k_maxSectionSize = UINT8_MAX;

constexpr uint8_t k_terminalMarker = 0xDD;
}

void DeltaCompression::Compress(
	const Collection::ArrayView<const uint8_t>& lastSeenBytes,
	const Collection::ArrayView<const uint8_t>& currentBytes,
	Collection::Vector<uint8_t>& outCompressedBytes)
{
	using namespace Internal_DeltaCompression;

	// If the last seen bytes are identical to the current bytes, all we serialize is k_identicalBytesMarker.
	if (lastSeenBytes.Size() == currentBytes.Size()
		&& memcmp(lastSeenBytes.begin(), currentBytes.begin(), currentBytes.Size()) == 0)
	{
		Mem::LittleEndian::Serialize(k_identicalBytesMarker, outCompressedBytes);
		return;
	}

	// To decompress later, we need to know the size before compression.
	Mem::LittleEndian::Serialize(static_cast<uint16_t>(currentBytes.Size()), outCompressedBytes);

	// We delta compress by searching for runs of identical bytes. To encode this, the compressed bytes consist of three
	// types of sections. A section begins with a two byte marker: a type and a size.

	// An unchanged section's size indicates how many bytes to read from the previous transmission.
	// An unchanged section ends immediately after its size; they are always 2 bytes.
	// A changed section's size indicates how many bytes to read from the current transmission.
	// A changed section ends size bytes after its marker.
	// A trailing section's size indicates how many bytes to read from the current transmission.
	// A trailing section ends size bytes after its marker.

	// Find runs of identical bytes and encode them as unchanged sections. Encode all other bytes in the overlapping
	// byte range as changed sections. Encode all bytes following the overlapping range in trailing sections.
	const size_t minByteCount =
		(lastSeenBytes.Size() < currentBytes.Size()) ? lastSeenBytes.Size() : currentBytes.Size();
	size_t i = 0;
	for (; i < minByteCount; /* CONTROLLED IN LOOP */)
	{
		const size_t rewindI = i;
		for (size_t j = 0; j < k_maxSectionSize && i < minByteCount; ++j, ++i)
		{
			if (lastSeenBytes[i] != currentBytes[i])
			{
				break;
			}
		}
		const size_t unchangedRunEnd = i;
		const size_t unchangedRunLength = unchangedRunEnd - rewindI;
		AMP_FATAL_ASSERT(unchangedRunLength <= k_maxSectionSize, "Sections can't exceed 256 bytes!");

		// An unchanged run is only worth encoding if its longer than a section marker.
		if (unchangedRunLength > 2)
		{
			outCompressedBytes.Add(k_unchangedSectionTypeID);
			outCompressedBytes.Add(static_cast<uint8_t>(unchangedRunLength));
			// i is already in the right place.
			continue;
		}

		// Rewind i to the start of this iteration.
		i = rewindI;

		// Step i forward until the 4 bytes after i are identical.
		bool foundNextUnchangedRun = false;
		for (size_t j = 0; j < k_maxSectionSize && i < (minByteCount - 3); ++j, ++i)
		{
			if (memcmp(lastSeenBytes.begin() + i, currentBytes.begin() + i, 3) == 0)
			{
				foundNextUnchangedRun = true;
				break;
			}
		}

		// A changed run is only added if the next unchanged run was found. If we reached the end of the overlapping
		// byte range, we just encode these bytes in a trailing section.
		if (!foundNextUnchangedRun)
		{
			// Rewind i before adding the trailing section.
			i = rewindI;
			break;
		}

		const size_t changedRunEnd = i;
		const size_t changedRunLength = changedRunEnd - rewindI;
		AMP_FATAL_ASSERT(changedRunLength <= k_maxSectionSize, "Sections can't exceed 256 bytes!");

		outCompressedBytes.Add(k_changedSectionTypeID);
		outCompressedBytes.Add(static_cast<uint8_t>(changedRunLength));
		outCompressedBytes.AddAll({ currentBytes.begin() + rewindI, changedRunLength });

		// i is already in the right place.
	}

	// Add trailing sections to ensure we don't drop any bytes from outside the overlapping range.
	while (i < currentBytes.Size())
	{
		const size_t remainingBytes = currentBytes.Size() - i;
		const size_t sectionSize = (remainingBytes < k_maxSectionSize) ? remainingBytes : k_maxSectionSize;

		outCompressedBytes.Add(k_trailingSectionTypeID);
		outCompressedBytes.Add(static_cast<uint8_t>(sectionSize));
		outCompressedBytes.AddAll({ currentBytes.begin() + i, sectionSize });

		i += sectionSize;
	}

	// Write a terminator marker.
	Mem::LittleEndian::Serialize(k_terminalMarker, outCompressedBytes);
}

bool DeltaCompression::TryDecompress(
	const Collection::ArrayView<const uint8_t>& lastSeenBytes,
	const uint8_t*& compressedIter,
	const uint8_t* const compressedBytesEnd,
	Collection::ArrayView<uint8_t>& inOutDecompressedBytes)
{
	using namespace Internal_DeltaCompression;

	// The first two bytes are either k_identicalBytesMarker or the size before compression.
	const auto maybeFirstTwoBytes = Mem::LittleEndian::DeserializeUi16(compressedIter, compressedBytesEnd);
	if (!maybeFirstTwoBytes.second)
	{
		return false;
	}

	// If the first two bytes are k_identicalBytesMarker, the decompressed bytes are the lastSeenBytes.
	if (maybeFirstTwoBytes.first == k_identicalBytesMarker)
	{
		if (lastSeenBytes.Size() > inOutDecompressedBytes.Size())
		{
			AMP_LOG_ERROR("Insufficient capacity to copy lastSeenBytes.");
			return false;
		}
		memcpy(inOutDecompressedBytes.begin(), lastSeenBytes.begin(), lastSeenBytes.Size());
		inOutDecompressedBytes = { inOutDecompressedBytes.begin(), lastSeenBytes.Size() };
		return true;
	}

	// Decompress in sections.
	const uint16_t numBytesBeforeCompression = maybeFirstTwoBytes.first;
	if (numBytesBeforeCompression > inOutDecompressedBytes.Size())
	{
		AMP_LOG_ERROR("Insufficient capacity to perform delta-decompression.");
		return false;
	}
	uint8_t* outIter = inOutDecompressedBytes.begin();
	inOutDecompressedBytes = { outIter, numBytesBeforeCompression };

	// Delta compressed bytes are divided into sections, each of which has a two byte header:
	// a one byte section type ID and a one byte size.
	const uint8_t* lastSeenIter = lastSeenBytes.begin();
	const uint8_t* const lastSeenEnd = lastSeenBytes.end();
	uint8_t sectionTypeID = k_terminalMarker;
	while ((compressedIter + 1) < compressedBytesEnd)
	{
		sectionTypeID = *(compressedIter++);
		if (sectionTypeID == k_terminalMarker)
		{
			break;
		}
		const uint8_t sectionSizeInBytes = *(compressedIter++);

		if (sectionTypeID == k_unchangedSectionTypeID)
		{
			if ((lastSeenIter + sectionSizeInBytes) > lastSeenEnd)
			{
				AMP_LOG_ERROR("Unexpectedly encountered the end of the last seen bytes during delta-decompression.");
				return false;
			}

			memcpy(outIter, lastSeenIter, sectionSizeInBytes);
			outIter += sectionSizeInBytes;
			lastSeenIter += sectionSizeInBytes;
		}
		else if (sectionTypeID == k_changedSectionTypeID)
		{
			memcpy(outIter, compressedIter, sectionSizeInBytes);
			outIter += sectionSizeInBytes;
			lastSeenIter += sectionSizeInBytes;
			compressedIter += sectionSizeInBytes;
		}
		else if (sectionTypeID == k_trailingSectionTypeID)
		{
			memcpy(outIter, compressedIter, sectionSizeInBytes);
			outIter += sectionSizeInBytes;
			compressedIter += sectionSizeInBytes;
		}
		else
		{
			AMP_LOG_ERROR(
				"Encountered unrecognized section type [%u] when delta-decompressing.",
				static_cast<uint32_t>(sectionTypeID));
			return false;
		}
	}

	// The loop above has a lookahead exit condition. If it ends due to that, ensure that it ended at a terminal marker.
	// Otherwise, assert that it found a terminal marker.
	if (compressedIter < compressedBytesEnd && sectionTypeID != k_terminalMarker)
	{
		AMP_LOG_ERROR("Delta-decompression failed to consume the correct number of bytes.");
		return false;
	}
	else
	{
		AMP_ASSERT(*(compressedIter - 1) == k_terminalMarker,
			"The above loop should have exited after finding a terminal marker!");
	}

	return true;
}
}
