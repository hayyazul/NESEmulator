#include "PPUDebug.h"

#include <iostream>
#include <iomanip>

PPUDebug::PPUDebug() : PPU() {
}

PPUDebug::PPUDebug(Memory* VRAM, Memory* CHRDATA) : PPU(VRAM, CHRDATA) {
}

PPUDebug::~PPUDebug()
{
}

void PPUDebug::displayNametable(int table) {
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
