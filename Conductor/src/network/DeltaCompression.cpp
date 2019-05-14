#include <network/DeltaCompression.h>

#include <collection/ArrayView.h>
#include <collection/Vector.h>

namespace Network
{
namespace Internal_DeltaCompression
{
constexpr uint8_t k_unchangedSectionTypeID = 0x0F;
constexpr uint8_t k_changedSectionTypeID = 0xF0;
constexpr uint8_t k_trailingSectionTypeID = 0xAA;
constexpr uint8_t k_maxSectionSize = UINT8_MAX;
}

void DeltaCompression::Compress(
	const Collection::ArrayView<const uint8_t>& lastSeenBytes,
	const Collection::ArrayView<const uint8_t>& currentBytes,
	Collection::Vector<uint8_t>& outCompressedBytes)
{
	using namespace Internal_DeltaCompression;

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
}

bool DeltaCompression::TryDecompress(
	const Collection::ArrayView<const uint8_t>& lastSeenBytes,
	const Collection::ArrayView<const uint8_t>& compressedBytes,
	Collection::Vector<uint8_t>& outDecompressedBytes)
{
	using namespace Internal_DeltaCompression;

	// Delta compressed bytes are divided into sections, each of which has a two byte header:
	// a one byte section type ID and a one byte size.
	const uint8_t* lastSeenIter = lastSeenBytes.begin();
	const uint8_t* const lastSeenEnd = lastSeenBytes.end();

	const uint8_t* compressedIter = compressedBytes.begin();
	const uint8_t* const compressedEnd = compressedBytes.end();
	while ((compressedIter + 1) < compressedEnd)
	{
		const uint8_t sectionTypeID = *(compressedIter++);
		const uint8_t sectionSizeInBytes = *(compressedIter++);

		if (sectionTypeID == k_unchangedSectionTypeID)
		{
			if ((lastSeenIter + sectionSizeInBytes) > lastSeenEnd)
			{
				AMP_LOG_ERROR("Unexpectedly encountered the end of the last seen bytes during delta-decompression.");
				return false;
			}

			outDecompressedBytes.AddAll({ lastSeenIter, sectionSizeInBytes });
			lastSeenIter += sectionSizeInBytes;
		}
		else if (sectionTypeID == k_changedSectionTypeID)
		{
			outDecompressedBytes.AddAll({ compressedIter, sectionSizeInBytes });
			lastSeenIter += sectionSizeInBytes;
			compressedIter += sectionSizeInBytes;
		}
		else if (sectionTypeID == k_trailingSectionTypeID)
		{
			outDecompressedBytes.AddAll({ compressedIter, sectionSizeInBytes });
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

	if (compressedIter != compressedEnd)
	{
		AMP_LOG_ERROR("Delta-decompression failed to consume the correct number of bytes.");
		return false;
	}

	return true;
}
}
