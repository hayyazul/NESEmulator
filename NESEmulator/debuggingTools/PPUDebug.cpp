#include "PPUDebug.h"

#include "../globals/helpers.hpp"
#include <iostream>
#include <iomanip>

PPUDebug::PPUDebug() : PPU() {
}

PPUDebug::PPUDebug(Memory* VRAM, Memory* CHRDATA) : PPU(VRAM, CHRDATA) {
}

PPUDebug::~PPUDebug()
{
}

void PPUDebug::displayNametable(int table) const {
	if (table < 0 || table > 3) {
		return;
	}

	uint16_t startAddr = TABLE_SIZE_IN_BYTES * table;

	for (uint16_t i = startAddr; i < startAddr + TABLE_WIDTH * TABLE_HEIGHT; ++i) {
		if (i % TABLE_WIDTH == 0) {
			std::cout << std::endl;
		}
		std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)this->VRAM->getByte(i) << ' ';
	}
	std::cout << std::endl << "--- Attribute Table ---" << std::endl;
	for (uint16_t i = startAddr + TABLE_WIDTH * TABLE_HEIGHT; i < startAddr + TABLE_SIZE_IN_BYTES; ++i) {
		if (i % 8 == 0) {
			std::cout << std::endl;
		}
		std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)this->VRAM->getByte(i) << ' ';
	}
}

std::array<uint8_t, TABLE_SIZE_IN_BYTES> PPUDebug::getNametable(int table) const {
	std::array<uint8_t, TABLE_SIZE_IN_BYTES> nametableData{};
	uint16_t startAddr = TABLE_SIZE_IN_BYTES * table;  // First get the starting address of the table.

	for (uint16_t i = 0; i < TABLE_SIZE_IN_BYTES; ++i) {
		nametableData.at(i) = this->VRAM->getByte(startAddr + i);
	}

	return nametableData;
}

void PPUDebug::displayPattern(uint8_t patternId, bool patternTable) const {
	// First, get the pattern.
	std::array<uint8_t, PATTERN_SIZE_IN_BYTES> pattern = this->getPattern(patternId, patternTable);

	unsigned int pixel;  // Can be 0, 1, 2, or 3.
	uint8_t lowerBitPlane, upperBitPlane;
	// Iterate through the 8 byte-pairs.
	for (unsigned int i = 0; i < 8; ++i) {
		lowerBitPlane = pattern.at(i);
		upperBitPlane = pattern.at(i + 0x8);
		// Then, iterate through the 8 bit-pairs in these.
		for (int j = 7; j >= 0; --j) {
			pixel = getBit(lowerBitPlane, j) + (getBit(upperBitPlane, j) << 1);

			// Lastly, display the pixel.
			std::cout << std::dec << pixel << ' ';
		}

		// Go to the next row after finishing the current one.
		std::cout << std::endl;
	}
}

std::array<uint8_t, PATTERN_TABLE_SIZE_IN_BYTES> PPUDebug::getPatternTable(bool table) const {
	// First, get the address of the pattern table.
	uint16_t patternTableAddr = (uint16_t)table << 12;
	uint16_t patternAddr;
	
	std::array<uint8_t, PATTERN_TABLE_SIZE_IN_BYTES> patternTableData{};

	unsigned int pixel;  // Can be 0, 1, 2, or 3.
	uint8_t bitPlane;  // A row of 8 bits indicating the color of a pixel (can either be a low or high bit; two bit planes decide the color).
	// Iterate through the 256 patterns.
	for (unsigned int i = 0; i < 256; ++i) {
		patternAddr = patternTableAddr + (i * PATTERN_SIZE_IN_BYTES);
		for (unsigned int j = 0; j < 16; ++j) {
			bitPlane = this->CHRDATA->getByte(patternAddr + j);
			auto a = i * 16 + j;
			patternTableData.at(i * 16 + j) = bitPlane;
		}
	}

	return patternTableData;
}

std::array<uint8_t, PATTERN_SIZE_IN_BYTES> PPUDebug::getPattern(uint8_t patternId, bool table) const {
	uint16_t patternTableAddr = (uint16_t)table << 12;
	uint16_t patternAddr = patternTableAddr + (patternId * PATTERN_SIZE_IN_BYTES);
	std::array<uint8_t, PATTERN_SIZE_IN_BYTES> pattern;

	// Loop through the rows of the pattern at the given address.
	uint8_t bitPlane;  // A row of 8 bits indicating the color of a pixel (can either be a low or high bit; two bit planes decide the color).
	for (unsigned int i = 0; i < 16; ++i) {
		bitPlane = this->CHRDATA->getByte(patternAddr + i);
		pattern.at(i) = bitPlane;
	}

	return pattern;
}

std::array<uint8_t, PALETTE_RAM_SIZE_IN_BYTES> PPUDebug::getPalette() {
	const uint16_t paletteAddress = 0x3f00;
	std::array<uint8_t, PALETTE_RAM_SIZE_IN_BYTES> paletteData;
	for (uint8_t i = 0; i < PALETTE_RAM_SIZE_IN_BYTES; ++i) {
		paletteData.at(i) = this->databus.read(paletteAddress + i);
	}

	return paletteData;
}

void PPUDebug::dumpOAMData(unsigned int lineSize) const {
	const int sizeOfOAMData = 0x100;  // There are 256 bytes in OAM data representing 64 sprites each defined w/ 4 bytes.
	uint8_t OAMByte;
	for (int i = 0; i < sizeOfOAMData; ++i) {
		if (i % lineSize == 0 && i) {
			std::cout << std::endl;
		}
		OAMByte = this->OAM.getByte(i);
		std::cout << displayHex(OAMByte, 2) << ' ';	
	}
}
