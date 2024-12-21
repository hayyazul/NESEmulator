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

void PPUDebug::displayPattern(bool patternTable, uint8_t pattern) const {
	// First, get the address of the pattern.
	uint16_t patternAddr = (uint16_t)patternTable << 12;
	patternAddr += (uint16_t)pattern << 4;
	// The address will be of the form 0tppn
	// t = pattern table (0 or 1); pp = pattern id; n = 0-7: lower bits of pattern, 8-f upper bits of pattern.

	unsigned int pixel;  // Can be 0, 1, 2, or 3.
	uint8_t lowerBitPlane, upperBitPlane;
	// Iterate through the 8 byte-pairs.
	for (unsigned int i = 0; i < 8; ++i) {
		lowerBitPlane = this->CHRDATA->getByte(i);
		upperBitPlane = this->CHRDATA->getByte(i + 0x8);
		// Then, iterate through the 8 bit-pairs in these.
		for (unsigned int j = 0; j < 8; ++j) {
			pixel = getBit(lowerBitPlane, i) + (getBit(upperBitPlane, i) << 1);

			// Lastly, display the pixel.
			std::cout << std::dec << pixel << ' ';
		}

		// Go to the next row after finishing the current one.
		std::cout << std::endl;
	}
}
