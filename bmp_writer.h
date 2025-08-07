/* Takes a 2-D vector of float3 structs representing pixel data and writes it to a BitMap file. */
#pragma once
#include <filesystem>
#include <cstdint>
//#include "vec.h" // Should be removed to increase portability

using namespace vec;

#pragma pack(push, 1) // (tell compiler to not align data here)
// Bitmap file header (empty data by default)
struct BmpHeader {
	char bitmapSignatureBytes[2] = { 'B', 'M' }; // bitmap header start
	uint32_t sizeOfBitmapFile = 54; // size of header plus pixel data (defaults to header only)
	uint32_t reservedBytes = 0;
	uint32_t pixelDataOffset = 54; // offset of pixel data from file start

    // Construct with pixel data, defaults to 32 bit color depth
	BmpHeader(int pixelDataSize) {
		sizeOfBitmapFile = 54 + pixelDataSize;
	}

    // Construct with pixel data and color depth
};

// Bitmap file info header (empty data by default)
struct BmpInfoHeader {
    uint32_t sizeOfThisHeader = 40;
    int32_t width = 0; // in pixels (defaults to empty)
    int32_t height = 0; // in pixels (defaults to empty)
    uint16_t numberOfColorPlanes = 1; // must be 1
    uint16_t colorDepth = (8 * 4); // 3 bytes per channel, plus one for alignment
    uint32_t compressionMethod = 0; // RGB format, no compression
    uint32_t rawBitmapDataSize = 0; // generally ignored
    int32_t horizontalResolution = 0; // in pixel per meter
    int32_t verticalResolution = 0; // in pixel per meter
    uint32_t colorTableEntries = 0;
    uint32_t importantColors = 0;

    BmpInfoHeader(int w, int h) {
        width = w;
        height = h;
    }
};
#pragma pack(pop) // (tell compiler to align data again)

// Gets size of all elements (x * y) in 2D vector
int sizeofImage(const std::vector<std::vector<float3>> &image)
{
	int size = 0;
	for (std::vector<float3> row : image) {
		size += row.size();
	}
	return size;
}

// Writes image in bitmap format to file by name
void WriteImageToBmp(const std::vector<std::vector<float3>> &image, const std::string &name, const int &frameNum = -1)
{
	int pixelSize = sizeofImage(image) * 4; // 3 bytes per channel, plus one for alignment
	int imageWidth = image[0].size();
	int imageHeight = image.size();

	// Create file headers
	BmpHeader bmpHeader{ pixelSize };
	BmpInfoHeader bmpInfoHeader{ imageWidth, imageHeight };

	// Open file in binary mode
	std::string fileName;
	if (frameNum < 0) {
		fileName = ("renders/" + name + ".bmp");
	}
	else {
		std::filesystem::create_directory(std::filesystem::current_path().string() + "/renders/" + name);
		fileName = "renders/" + name + "/frame_" + std::to_string(frameNum) + ".bmp";
	}
	std::ofstream imageFile((fileName), std::ios::binary);

	// Make sure file is open before attempting any writes
	if (!imageFile.is_open()) { throw "File failed to open!"; }

	// Write file headers
	imageFile.write((char *)&bmpHeader, sizeof(BmpHeader));
	imageFile.write((char *)&bmpInfoHeader, sizeof(bmpInfoHeader));

	// Combine all pixel data before writing
	std::vector<uint8_t> pixelData;
	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {
			uint8_t rChannel = (uint8_t)(image[y][x].r * 255);
			uint8_t gChannel = (uint8_t)(image[y][x].g * 255);
			uint8_t bChannel = (uint8_t)(image[y][x].b * 255);
			uint8_t alpha = 0;
			pixelData.push_back(bChannel);	// BMP writes blue first,
			pixelData.push_back(gChannel);	// green second,
			pixelData.push_back(rChannel);	// red third,
			pixelData.push_back(alpha);		// alpha last
		}
	}

	// Write pixeldata to file
	imageFile.write((char *)pixelData.data(), pixelData.size());

	imageFile.close();
}