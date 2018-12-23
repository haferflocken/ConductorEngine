#include <image/Pixel1Image.h>

#include <file/FullFileReader.h>
#include <mem/DeserializeLittleEndian.h>

namespace Image
{
bool Pixel1Image::TryLoad(const File::Path& filePath, Pixel1Image* destination)
{
	const std::string rawBMP = File::ReadFullTextFile(filePath);
	if (rawBMP.size() < 30)
	{
		return false;
	}

	const uint8_t* dataIter = reinterpret_cast<const uint8_t*>(rawBMP.data());
	const uint8_t* const dataEnd = dataIter + rawBMP.size();

	// 14 bytes:
	const uint16_t signature = Mem::LittleEndian::DeserializeUi16(dataIter, dataEnd).first;
	const uint32_t fileSize = Mem::LittleEndian::DeserializeUi32(dataIter, dataEnd).first;
	dataIter += 4;
	const uint32_t fileOffsetToPixels = Mem::LittleEndian::DeserializeUi32(dataIter, dataEnd).first;

	if (fileOffsetToPixels >= rawBMP.size())
	{
		return false;
	}

	// 16 bytes:
	const uint32_t dibHeaderSize = Mem::LittleEndian::DeserializeUi32(dataIter, dataEnd).first;
	const uint32_t widthPixels = Mem::LittleEndian::DeserializeUi32(dataIter, dataEnd).first;
	const int32_t signedHeightPixels = Mem::LittleEndian::DeserializeI32(dataIter, dataEnd).first;
	dataIter += 2;
	const uint16_t bitsPerPixel = Mem::LittleEndian::DeserializeUi16(dataIter, dataEnd).first;
	
	// Validate format.
	if (signedHeightPixels < 0 || bitsPerPixel != 1)
	{
		return false;
	}
	const uint32_t heightPixels = static_cast<uint32_t>(signedHeightPixels);

	// Validate size.
	const uint32_t requiredWidthBytes = (widthPixels + 7) / 8;
	const uint32_t widthBytes = requiredWidthBytes + (requiredWidthBytes % 4);

	const uint32_t pixelsEndFileOffset = fileOffsetToPixels + (widthBytes * heightPixels);
	if (pixelsEndFileOffset > rawBMP.size())
	{
		return false;
	}

	// Read the image data.
	destination = new(destination) Pixel1Image(widthPixels, heightPixels);
	
	const uint8_t* const pixelDataStart = reinterpret_cast<const uint8_t*>(rawBMP.data()) + fileOffsetToPixels;
	for (uint32_t y = 0; y < heightPixels; ++y)
	{
		const uint8_t* const rowDataStart = pixelDataStart + (y * widthBytes);
		for (uint32_t x = 0; x < widthPixels; ++x)
		{
			const size_t byteIndex = x / 8;
			const size_t bitIndex = 7 - (x % 8);
			const uint8_t rowByte = rowDataStart[byteIndex];
			const bool isPixelSet = (rowByte & (1 << bitIndex)) != 0;
			destination->SetValue(x, y, isPixelSet);
		}
	}
	
	return true;
}
}
